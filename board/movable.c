#include "movable.h"
#include <stdbool.h>
#include <stdio.h>
#define MAX_JUMPS 6


bool jump_enabled = true;
bool first_step = true;
/* ───────── 萬用鄰居函式 ─────────(取得方向為 dir 的鄰居的地址)
   dir：0=NW 1=NE 2=W 3=E 4=SW 5=SE  六種方向，NW, NE, W, E, SW, SE         */
/* 0=NW 1=NE 2=W 3=E 4=SW 5=SE — 完整考慮列縮排差 + 奇偶半格 */
void get_neighbor(int r, int c, int dir, int *nr, int *nc)
{
    //上下兩列縮排差
    int diff_up   = (r > 0)        ? rowOffsetTable[r] - rowOffsetTable[r-1] : 0;//跟上一列的縮排差，由上而下為 1 0 1 -1 1 -1 1 -1 0 -1
    int diff_down = (r < ROWS - 1) ? rowOffsetTable[r] - rowOffsetTable[r+1] : 0;//跟下一列的縮排差，由上而下為 -1 0 -1 1 -1 1 -1 1 0 1
    bool even = (r % 2 == 0);

    switch (dir) {
        //上一列
        case 0: //NW
            *nr = r - 1;
            *nc = c - (even ? 1 : 0)//偶數列要相對最長的列(奇數列)縮半格                                                                   
                    + diff_up;                
            break;
        case 1: /* NE */
            *nr = r - 1;
            *nc = c + (even ? 0 : 1)          
                    + diff_up;
            break;

        //同一列//不用考慮上下列縮排差
        case 2: /* W  */ *nr = r; *nc = c - 1; break;
        case 3: /* E  */ *nr = r; *nc = c + 1; break;

        //下一列
        case 4: /* SW */
            *nr = r + 1;
            *nc = c - (even ? 1 : 0)
                    + diff_down;
            break;
        case 5: /* SE */
            *nr = r + 1;
            *nc = c + (even ? 0 : 1)
                    + diff_down;
            break;
    }
}


/*
static bool surrounding_safe(int r, int c)//(r, c) 的六個方位的鄰居都沒有 monster，則 true
{
    for (int d = 0; d < 6; d++) {
        int nr, nc;
        get_neighbor(r, c, d, &nr, &nc);

        //超出棋盤 → 視為安全 
        if (nr < 0 || nr >= ROWS)                   continue;
        if (nc < 0 || nc >= rowLengths[nr])        continue;

        if (is_monster_at(nr, nc))                  return false;
    }
    return true;
}*/

static void try_add_adjacent(int nr, int nc)//將座標 (nr, nc) 加入可以移動到的鄰點
{
    if (!first_step) return;// 只允許第一步
    if (is_path_blocked(player.r, player.c, nr, nc)) return;

    if (nr >= 0 && nr < ROWS &&
        nc >= 0 && nc < rowLengths[nr] &&
        !is_monster_at(nr, nc))//沒有越界，鄰居也不是 monster
    {
        if (movable_cnt < MAX_MOVES) {
            movable_tiles[movable_cnt++] = (Pos){nr, nc};
        }
    }
}

//判斷 (r,c) 是否在 movable_tiles[] 
int is_movable(int r, int c)
{
    for (int i = 0; i < movable_cnt; i++)
        if (movable_tiles[i].r == r && movable_tiles[i].c == c)
            return 1;
    return 0;
}


Pos movable_tiles[MAX_MOVES];//紀錄可以移動到的鄰點(至多六個)
int movable_cnt = 0;

Pos jump_tiles[MAX_JUMPS];//紀錄可以跳動到的點
int jump_cnt = 0; 

//將座標 (r, c) 加入可以鏡射跳到的點
static void add_jump(int r, int c)
{
    if (jump_cnt >= MAX_JUMPS) return;
    for (int i = 0; i < jump_cnt; i++)
        if (jump_tiles[i].r == r && jump_tiles[i].c == c) return;
    jump_tiles[jump_cnt++] = (Pos){r, c};
}

void update_movable_tiles(void)//更新可以移動到的鄰點，或可以鏡射跳到的格子點
{
    movable_cnt = 0;
    jump_cnt    = 0;

    for (int d = 0; d < 6; d++) { //檢查六個方向有無平移或鏡射跳可能
        //一步可移格//座標 (nr, nc) 有無平移可能
        int nr, nc;
        get_neighbor(player.r, player.c, d, &nr, &nc);
        try_add_adjacent(nr, nc);  //只在 first_step 時生效

        if (!jump_enabled) continue;

        //計算與怪物距離 //座標 (tr, tc) 有無鏡射跳可能
        int tr = nr, tc = nc;//從第一鄰格開始前探
        int distBefore = 1;
        while (tr >= 0 && tr < ROWS &&
               tc >= 0 && tc < rowLengths[tr] &&
               !is_monster_at(tr, tc))//沒有越界且不是怪物
        {
            distBefore++;//怪物前方空格數
            int pr = tr, pc = tc;
            get_neighbor(pr, pc, d, &tr, &tc);
        }

        //若沒遇到怪物就換下一方向 
        if (!(tr >= 0 && tr < ROWS &&
              tc >= 0 && tc < rowLengths[tr] &&
              is_monster_at(tr, tc)))
            continue;

        //嘗試對稱跳 //有遇到怪物
        int stepsAfter = distBefore; //遇到怪物後需走的格數
        int jr, jc;
        get_neighbor(tr, tc, d, &jr, &jc);   //怪物後第一格的座標

        bool pathClear = true;
        for (int s = 1; s <= stepsAfter; s++) {
            //檢查每一步皆在棋盤且無怪物
            if (!(jr >= 0 && jr < ROWS &&
                  jc >= 0 && jc < rowLengths[jr]) ||
                is_monster_at(jr, jc)) //沒有越界或是遇到怪物
            {
                pathClear = false;
                break;
            }
            if (s < stepsAfter) { //距離想鏡射的怪物中間還有其他怪物
                int pr = jr, pc = jc;
                get_neighbor(pr, pc, d, &jr, &jc);//
            }
        }

        if (pathClear && !is_path_blocked(player.r, player.c, jr, jc)) {
            add_jump(jr, jc); //路徑上若又怪物，且鏡射位置沒有被 block => 可以跳躍至 (jr, jc) 上
        }
    }
}