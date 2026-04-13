#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "../GAME_ASSERT.h"
#include "skill.h"                 
#include "../global.h"
#include "../board/hexboard.h"
#include "../monster/monster.h"
#include "../player/player.h"
#include "../ui/flash.h" 
#include"../ui/bubble.h"
#include"../ui/cookie_rain.h"
#include"../ui/sugar_dust.h"
#include"../ui/peanut_bomb.h"
#include "../ui/candy_drop.h"

// 全域狀態
SelectedSkill selected_skills[MAX_SELECTED_SKILLS];
int           selected_count   = 0;
Skill         all_skills[SKL_COUNT];
Skill        *displayed_skills[3];

bool          show_skill_scene = false;
ALLEGRO_FONT *skill_font       = NULL;


// 還沒做的技能先用 TODO
#define TODO(s) do{ printf("[TODO] %s\\n", (s)); }while(0)


// skill 0
// jl_strawberry 用的幾何小工具
static int find_seg_direction(int sr, int sc, int er, int ec);
static void hex_to_screen(int r, int c, float *x, float *y) {
    float W = get_hex_w(), V = get_hex_v();
    float row_off = rowOffsetTable[r]*W + (r%2)*(W/2);
    float center_x = rowOffsetTable[5]*W + 5*W + (5%2)*(W/2);
    float center_y = 5*V;
    float ox = WIDTH/2  - center_x;
    float oy = HEIGHT/2 - center_y;
    *x = ox + row_off + c*W;
    *y = oy + r*V;
}
static int orientation(float ax,float ay,float bx,float by, float cx,float cy) {
    float v = (bx-ax)*(cy-ay) - (by-ay)*(cx-ax);
    return (v>0)?1: (v<0)?2:0;
}
static bool segments_intersect(float ax,float ay,float bx,float by,
                               float cx,float cy,float dx,float dy) {
    int o1=orientation(ax,ay,bx,by, cx,cy);
    int o2=orientation(ax,ay,bx,by, dx,dy);
    int o3=orientation(cx,cy,dx,dy, ax,ay);
    int o4=orientation(cx,cy,dx,dy, bx,by);
    return (o1!=o2 && o3!=o4);
}
// 六個方向的反方向
static const int DIR_OPPOSITE[6] = { 5, 4, 3, 2, 1, 0 };
static inline int opposite_dir(int d) { return DIR_OPPOSITE[d]; }

// 同一排一路掃過去
static void extend_row_ray(int sr, int sc, int dmg) {

    add_flash_tile(sr, sc);
    printf("[Strawberry] row-ray origin (%d,%d)\n", sr, sc);
    // 起點有怪就先打
    if (is_monster_at(sr, sc)) {
        printf("[Strawberry] hit monster at origin (%d,%d) for %d dmg\n", sr, sc, dmg);
        damage_monster_at(sr, sc, dmg);
    }

    // 往左掃
    for (int c = sc - 1; c >= 0; --c) {
        add_flash_tile(sr, c);
        printf("[Strawberry] row-ray passes (%d,%d)\n", sr, c);
        if (is_monster_at(sr, c)) {
            printf("[Strawberry] hit monster at (%d,%d) for %d dmg\n", sr, c, dmg);
            damage_monster_at(sr, c, dmg);
        }
    }
    // 往右掃
    for (int c = sc + 1; c < rowLengths[sr]; ++c) {
        add_flash_tile(sr, c);
        printf("[Strawberry] row-ray passes (%d,%d)\n", sr, c);
        if (is_monster_at(sr, c)) {
            printf("[Strawberry] hit monster at (%d,%d) for %d dmg\n", sr, c, dmg);
            damage_monster_at(sr, c, dmg);
        }
    }
}




// 沿著一條線往兩邊延伸
static void extend_ray(int sr, int sc, int er, int ec, int dmg) {
    // 先閃起點
    add_flash_tile(sr, sc);
    printf("[Strawberry] line origin (%d,%d)\n", sr, sc);
    // 起點有怪就先打
    if (is_monster_at(sr, sc)) {
        printf("[Strawberry] hit monster at (%d,%d) for %d dmg\n", sr, sc, dmg);
        damage_monster_at(sr, sc, dmg);
    }

    // 算這段的方向和反方向
    int d  = find_seg_direction(sr, sc, er, ec);
    int db = opposite_dir(d);

    // 往正方向一路延伸到邊界
    {
        int r = sr, c = sc;
        while (true) {
            int nr, nc;
            get_neighbor(r, c, d, &nr, &nc);
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= rowLengths[nr])
                break;
            add_flash_tile(nr, nc);
            printf("[Strawberry] ray passes (%d,%d)\n", nr, nc);
            if (is_monster_at(nr, nc)) {
                printf("[Strawberry] hit monster at (%d,%d) for %d dmg\n",
                       nr, nc, dmg);
                damage_monster_at(nr, nc, dmg);
            }
            r = nr;  c = nc;
        }
    }

    // 再往反方向延伸到邊界
    {
        int r = sr, c = sc;
        while (true) {
            int nr, nc;
            get_neighbor(r, c, db, &nr, &nc);
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= rowLengths[nr])
                break;
            add_flash_tile(nr, nc);
            printf("[Strawberry] ray passes (%d,%d)\n", nr, nc);
            if (is_monster_at(nr, nc)) {
                printf("[Strawberry] hit monster at (%d,%d) for %d dmg\n",
                       nr, nc, dmg);
                damage_monster_at(nr, nc, dmg);
            }
            r = nr;  c = nc;
        }
    }
}

// 找出這段是哪個方向
static int find_seg_direction(int sr,int sc,int er,int ec){
    for(int d=0;d<6;d++){
        int r=sr, c=sc;
        while(true){
            int nr,nc; get_neighbor(r,c,d,&nr,&nc);
            if(nr==er && nc==ec) return d;
            if(nr<0||nr>=ROWS||nc<0||nc>=rowLengths[nr]) break;
            r=nr; c=nc;
        }
    }
    return -1;
}

static void jl_strawberry(Skill *s, int end_r, int end_c)
{
    
    float pct = 35.0f * (s->level);
    int base_dmg = (int)(player.atk * pct / 100.0f + 0.5f);

    printf("[DEBUG] jl_strawberry called, path_len=%d\n", path_len);
    for (int i = 0; i < path_len; i++) {
        printf("  path[%d] = (%d,%d)\n", i,
               move_path[i].r, move_path[i].c);
    }

    float px[MAX_PATH_LEN], py[MAX_PATH_LEN];
    for (int i = 0; i < path_len; i++) {
        hex_to_screen(move_path[i].r, move_path[i].c, &px[i], &py[i]);
        printf("  px[%d]=%.2f, py[%d]=%.2f\n", i, px[i], i, py[i]);
    }

    bool hit_seg[MAX_PATH_LEN] = {0};

    // 檢查所有不相鄰的線段有沒有交叉
    for (int i = 0; i + 1 < path_len; i++) {
        for (int j = i + 2; j + 1 < path_len; j++) {
            if (i == j - 1) continue;  // 共端点

            printf("[DEBUG] testing segments %d-%d vs %d-%d\n",
                   i, i+1, j, j+1);

            // 先算 orientation 方便 debug
            int o1 = orientation(px[i],   py[i],
                                 px[i+1], py[i+1],
                                 px[j],   py[j]);
            int o2 = orientation(px[i],   py[i],
                                 px[i+1], py[i+1],
                                 px[j+1], py[j+1]);
            int o3 = orientation(px[j],   py[j],
                                 px[j+1], py[j+1],
                                 px[i],   py[i]);
            int o4 = orientation(px[j],   py[j],
                                 px[j+1], py[j+1],
                                 px[i+1], py[i+1]);
            printf("    orientations: o1=%d, o2=%d, o3=%d, o4=%d\n",
                   o1, o2, o3, o4);

            // 再判斷有沒有真的相交
            bool inter = segments_intersect(px[i],   py[i],
                                            px[i+1], py[i+1],
                                            px[j],   py[j],
                                            px[j+1], py[j+1]);
            printf("    segments_intersect? %s\n",
                   inter ? "YES" : "no");

            if (inter) {
                printf("  >>> intersection at segments %d-%d × %d-%d!\n",
                       i, i+1, j, j+1);

                // 第 i 段如果第一次撞到，就放射線
                if (!hit_seg[i]) {
                      int sr = move_path[i].r;
                    int sc = move_path[i].c;
                    int er = move_path[i+1].r;
                    int ec = move_path[i+1].c;

                    if (sr == er) {
                        // 同一排就水平掃
                        extend_row_ray(sr, sc, base_dmg);
                    } else {
                        // 不是同一排就走斜線
                        int d1 = find_seg_direction(sr, sc, er, ec);
                        extend_ray(sr, sc, er, ec, base_dmg);
                    }
                    hit_seg[i] = true;
                }

                // 第 j 段也一樣處理
                if (!hit_seg[j]) {
    int sr = move_path[j].r;
    int sc = move_path[j].c;
    int er = move_path[j+1].r;
    int ec = move_path[j+1].c;

    if (sr == er) {
        // 同一排就水平掃
        extend_row_ray(sr, sc, base_dmg);
    } else {
        // 否則走斜線
        int d2 = find_seg_direction(sr, sc, er, ec);
        printf("    ray2 dir=%d\n", d2);
       extend_ray(sr, sc, er, ec, base_dmg);
    }
    hit_seg[j] = true;
}
            }
        }
    }

    // 最後印總傷害
    printf("[Strawberry Jelly Burst] Lv%d => %.0f%% of ATK = %d dmg\n",
           s->level, pct, base_dmg);
}



// skill 1 
static void jl_matcha_tw     (Skill *s, int r, int c) { TODO(s->name); } 
// skill 2
static void jl_sesame        (Skill *s, int r, int c) { TODO(s->name); } 
// skill 3
static void jl_mint(Skill *s, int r, int c) {
    // 每級多 24% 傷害
    float pct = 24.0f * s->level;
    int dmg = (int)(player.atk * pct / 100.0f + 0.5f);

    // 落點閃一下，再生泡泡
    add_flash_tile(r, c);
    printf("[%s] bubble at (%d,%d), damage %% = %.0f%% (dmg = %d)\n",
           s->name, r, c, pct, dmg);

    // 打周圍一圈
    for (int d = 0; d < 6; d++) {
        int nr, nc;
        get_neighbor(r, c, d, &nr, &nc);
        // 出界就跳過
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= rowLengths[nr])
            continue;
        spawn_mint_bubble(r, c, nr, nc);
        printf("[%s] hit at (%d,%d) for %d dmg\n", s->name, nr, nc, dmg);
        if (is_monster_at(nr, nc)) {
            damage_monster_at(nr, nc, dmg);
        }
    }
}

 
// skill 4
static void jl_cookie(Skill *s, int r, int c) {
    // 傷害倍率
    float pct = 50.0f * s->level;
    int dmg = (int)(player.atk * pct / 100.0f + 0.5f);

    // 在落點炸開
    add_flash_tile(r, c);
    printf("[%s] cookie explosion at (%d,%d), damage %% = %.0f%% (dmg = %d)\n",
           s->name, r, c, pct, dmg);

    // 用 BFS 找距離 2 內的格子
    // row 最長 11 格，這樣開夠用
    bool visited[ROWS][12] = {{0}};
    int  dist[ROWS][12]    = {{0}};
    typedef struct { int r, c; } Pos;
    Pos queue[ROWS*12];
    int head = 0, tail = 0;

    // 起點先丟進 queue
    visited[r][c] = true;
    dist[r][c]    = 0;
    queue[tail++] = (Pos){r, c};

    while (head < tail) {
        Pos p = queue[head++];
        int d0 = dist[p.r][p.c];
        if (d0 >= 2) continue;  // 超過距離 2 就不用再擴

        // 往六個方向擴
        for (int d = 0; d < 6; d++) {
            int nr, nc;
            get_neighbor(p.r, p.c, d, &nr, &nc);
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= rowLengths[nr]) 
                continue;
            if (visited[nr][nc]) 
                continue;

            visited[nr][nc] = true;
            dist[nr][nc]    = d0 + 1;
            queue[tail++]   = (Pos){nr, nc};

            // 只打距離 1 和 2 的格子，起點不打
            printf("[%s] hit at (%d,%d) for %d dmg (dist=%d)\n",
                   s->name, nr, nc, dmg, dist[nr][nc]);
            if (is_monster_at(nr, nc)) {
                damage_monster_at(nr, nc, dmg);
            }
        }
        cookie_center_r = r;
        cookie_center_c = c;
        spawn_cookie_rain(r, c);
    }
}

// skill 5
static void jl_matcha_core(Skill *s, int r, int c) {
    // 只有普攻擊殺時才會進來
    // 傷害倍率
    float pct = 45.0f * s->level;   // Lv1 45%, Lv2 90%...
    int dmg = (int)(player.atk * pct / 100.0f + 0.5f);

    add_flash_tile(r, c);
    printf("[%s] core exploded at (%d,%d) → %.0f%% ATK (%d dmg)\n",
           s->name, r, c, pct, dmg);

    for (int d = 0; d < 6; d++) {
        int nr, nc;
        get_neighbor(r, c, d, &nr, &nc);
        if (nr<0||nr>=ROWS||nc<0||nc>=rowLengths[nr]) continue;
        add_flash_tile(nr, nc);
        printf("[%s] hit at (%d,%d) for %d dmg\n", s->name, nr, nc, dmg);
        if (is_monster_at(nr, nc))
            damage_monster_at(nr, nc, dmg);
    }
}


// skill 6//未完成
static void jl_sugar_dust(Skill *s, int r, int c) {
    // 這裡只記路徑，傷害另外算
    if (path_len < 2) return;

    Pos prev = move_path[path_len - 2];
    int pr = prev.r, pc = prev.c;

    // 算方向
    int d = find_seg_direction(pr, pc, r, c);
    if (d < 0) {
        // 如果不是直線，只記前後兩格
        add_sugar_dust_tile(pr, pc);
        add_sugar_dust_tile(r, c);
        return;
    }

    // 如果是直線，就整段都記下來
    int tr = pr, tc = pc;
    while (1) {
        add_sugar_dust_tile(tr, tc);
        if (tr == r && tc == c) break;
        int nr, nc;
        get_neighbor(tr, tc, d, &nr, &nc);
        tr = nr; tc = nc;
    }
}




// 回合開始觸發
// skill 7


static void ts_nougat(Skill *s) {
    // 傷害倍率
    float pct = 30.0f * s->level;   // Lv1 45%, Lv2 90%...
    int dmg = (int)(player.atk * pct / 100.0f + 0.5f);

    int dropped = 0;
    // 找前三隻活著的怪丟炸彈
    for (int i = 0; i < MAX_MONSTERS && dropped < 3; i++) {
        // monsters 是怪物陣列
        if (!monsters[i].alive) continue;
        int mr = monsters[i].r;
        int mc = monsters[i].c;
        dropped++;

        printf("[%s] dropping nougat bomb at (%d,%d)\n", s->name, mr, mc);
        // 中心和周圍一圈都會吃傷害
        //add_flash_tile(mr, mc);
        spawn_peanut_bomb(mr, mc);
        if (is_monster_at(mr, mc)) {
            damage_monster_at(mr, mc, dmg);
            printf("  [%s] hit monster at (%d,%d) for %d dmg\n",
                   s->name, mr, mc, dmg);
        }
        for (int d = 0; d < 6; d++) {
            int nr, nc;
            get_neighbor(mr, mc, d, &nr, &nc);
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= rowLengths[nr]) 
                continue;
            
            if (is_monster_at(nr, nc)) {
                damage_monster_at(nr, nc, dmg);
                printf("  [%s] hit monster at (%d,%d) for %d dmg\n",
                       s->name, nr, nc, dmg);
            }
        }
    }

    printf("[%s] Nougat Bomb deployed on %d monsters\n", s->name, dropped);
}


// skill 8
          // for rand()


static void ts_caramel(Skill *s) {
    // 每級 12% 傷害
    float pct = 12.0f * s->level;
    int dmg = (int)(player.atk * pct / 100.0f + 0.5f);

    int drops = 0;
    while (drops < 3) {
        // 隨機挑一個合法格子
        int r = rand() % ROWS;
        int c = rand() % rowLengths[r];

        // 在這格丟糖球
        spawn_candy_drop(r, c);
        printf("[%s] caramel bomb dropped at (%d,%d)\n", s->name, r, c);

        // 中心格
        if (is_monster_at(r, c)) {
            damage_monster_at(r, c, dmg);
            printf("  [%s] hit monster at (%d,%d) for %d dmg\n",
                   s->name, r, c, dmg);
        }
        // 六個方向
        for (int d = 0; d < 6; d++) {
            int nr, nc;
            get_neighbor(r, c, d, &nr, &nc);
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= rowLengths[nr])
                continue;
            spawn_candy_drop(nr, nc);
            if (is_monster_at(nr, nc)) {
                damage_monster_at(nr, nc, dmg);
                printf("  [%s] hit monster at (%d,%d) for %d dmg\n",
                       s->name, nr, nc, dmg);
            }
        }
        drops++;
    }
    printf("[%s] dropped %d caramel bombs (%.0f%% ATK each)\n",
           s->name, drops, pct);
}


// skill 9
static void pv_sugar_rush(Skill *s) {
    // 每級加成
    float bonus = 0.2f * s->level;

    // 只有血量低於一半才開
    if ((float)player.hp / player.max_hp < 0.5f) {
        player.damage_multiplier = 1.0f + bonus;
        printf("[Sugar Rush] HP<50%%, damage ×%.2f\\n", player.damage_multiplier);
    } else {
        // 血回上去就把加成拿掉
        player.damage_multiplier = 1.0f;
    }
}
// skill 10

static void pv_snow_salt(Skill *s) {
    
    float bonus = 0.06f * s->level;
    // 直接改 combo 倍率
    player.damage_multiplier2 = 1.0f + bonus;
    printf("[%s] combo multiplier set to ×%.2f\n",
           s->name, player.damage_multiplier2);
}

// skill 11
static void pv_cream_lifesteal(Skill *s) {
    // 每級加吸血
    player.lifesteal = 0.04f * s->level;
    printf("[%s] lifesteal set to %.0f%% of damage\n",
           s->name, player.lifesteal * 100.0f);
}

// skill 12
static void pv_sweet_counter(Skill *s) {
    // 反擊倍率
    player.counter_multiplier = 0.5f * s->level;
    printf("[%s] Sweet Counter active: counter ×%.2f\n",
           s->name, player.counter_multiplier);
}

// skill 13
static void pv_cream_shield(Skill *s) {
    // 每級 4% 減傷
    player.damage_reduction = 0.04f * s->level;
    printf("[%s] Cream Shield active: damage reduced by %.0f%%\n",
           s->name, player.damage_reduction * 100.0f);
}

// skill 14
static void sp_fruit_heal    (Skill *s)
{
    int heal = player.max_hp / 2;
    player.hp = (player.hp + heal > player.max_hp) ? player.max_hp : player.hp + heal;
    printf("[Fruit Medica] healed %d HP\\n", heal);
} // 14

// 技能表
typedef struct {
    const char *name; 
    unsigned char r,g,b; 
    JumpLandCB jl; 
    TurnStartCB ts;
    TurnEndCB      te; 
    bool        available;
} SkillDef;
#define RGB(r,g,b) (unsigned char)r,(unsigned char)g,(unsigned char)b
static const SkillDef DEF[SKL_COUNT] = {
 [SKL_STRAWBERRY]      = { "Jelly Burst",  RGB(255, 80,120), NULL,  NULL   , jl_strawberry,true },
 [SKL_MATCHA_TORNADO]  = { "Tornado Cake",      RGB(100,200,100), jl_matcha_tw,   NULL  , NULL  ,false   },
 [SKL_SESAME_LINE]     = { "Sesame Line",        RGB(150,100, 50), jl_sesame,      NULL  , NULL  ,false   },
 [SKL_MINT_BUBBLE]     = { "Mint Bubble Puff",         RGB(120,255,255), jl_mint,        NULL  ,NULL  ,true     },
 [SKL_CREAM_COOKIE]    = { "Cream Filled Cookie",      RGB(255,220,180), jl_cookie,      NULL    ,NULL  ,true   },
 [SKL_MATCHA_CORE]     = { "Cream Core",        RGB( 80,160, 80), jl_matcha_core, NULL    ,NULL  ,true   },
 [SKL_NOUGAT_BOMB]     = { "Peanut Bombs",      RGB(230,200,120), NULL,           ts_nougat ,NULL  ,true },
 [SKL_CARAMEL_SHROOM]  = { "Caramel Bombs",     RGB(200,130, 60), NULL,           ts_caramel ,NULL  ,true},
 [SKL_SUGAR_DUST]      = { "Sugar Dust Trail",         RGB(245,245,245), jl_sugar_dust,  NULL , NULL   ,false   },
 [SKL_SUGAR_RUSH]      = { "Sugar Rush",               RGB(255,100,100), NULL,          pv_sugar_rush, NULL ,true    },
 [SKL_SNOW_SALT]       = { "Snow Frosting",       RGB(220,220,255), NULL,          pv_snow_salt   , NULL  ,true  },
 [SKL_CREAM_LIFESTEAL] = { "Cream Lifesteal",          RGB(255,230,200), NULL,          pv_cream_lifesteal, NULL ,true      },
 [SKL_SWEET_COUNTER]   = { "Sweet Counter",            RGB(255,150,150), NULL,          pv_sweet_counter , NULL ,true     },
 [SKL_CREAM_SHIELD]    = { "Cream Shield",             RGB(200,200,255), NULL,          pv_cream_shield  , NULL ,true    },
 [SKL_FRUIT_HEAL]      = { "Fruit Medica",             RGB(255, 50, 50), NULL,           NULL   , NULL ,true   },
};
//#define TEST_STRAWBERRY   1  
// 初始化
void init_skill_system(void)
{
    skill_font = al_create_builtin_font();
    GAME_ASSERT(skill_font, "Failed to load skill font");

    for (int i = 0; i < SKL_COUNT; ++i) {
        all_skills[i].id            = i;
        all_skills[i].name          = DEF[i].name;
        all_skills[i].color         = al_map_rgb(DEF[i].r, DEF[i].g, DEF[i].b);
        all_skills[i].level         = 1;
        all_skills[i].on_jump_land  = DEF[i].jl;
        all_skills[i].on_turn_start = DEF[i].ts;
        all_skills[i].on_turn_end   = DEF[i].te;
        all_skills[i].available = DEF[i].available;

    }
    #ifdef TEST_STRAWBERRY
    // 測試用：直接給 Strawberry 滿等
    selected_count = 1;
    selected_skills[0].skill  = &all_skills[SKL_STRAWBERRY];
    selected_skills[0].level  = MAX_SKILL_LEVEL;   // 6
    all_skills[SKL_STRAWBERRY].level = MAX_SKILL_LEVEL;
    printf("[TEST] Auto-select skill: %s Lv.%d\n",
           all_skills[SKL_STRAWBERRY].name,
           selected_skills[0].level);
#endif
#undef TEST_STRAWBERRY

#ifdef TEST_MINT
    selected_count = 1;
    selected_skills[0].skill  = &all_skills[SKL_MINT_BUBBLE];
    selected_skills[0].level  = MAX_SKILL_LEVEL;
    all_skills[SKL_MINT_BUBBLE].level = MAX_SKILL_LEVEL;
    printf("[TEST] Auto-select skill: %s Lv.%d\n",
           all_skills[SKL_MINT_BUBBLE].name,
           selected_skills[0].level);
#endif
//#define TEST_COOKIE 1
#ifdef TEST_COOKIE
    selected_count = 1;
    selected_skills[0].skill = &all_skills[SKL_CREAM_SHIELD];
    selected_skills[0].level = MAX_SKILL_LEVEL;
    all_skills[SKL_CREAM_SHIELD].level = MAX_SKILL_LEVEL;
    printf("[TEST] Auto-select skill: %s Lv.%d\n",
           all_skills[SKL_CREAM_SHIELD].name,
           selected_skills[0].level);
#endif
}

// 事件觸發
void trigger_on_jump_land(int r,int c){ 
    for(int i=0;i<selected_count;++i){ 
        Skill*s=selected_skills[i].skill; 
        if(s->on_jump_land) s->on_jump_land(s,r,c);
    } 
}
void trigger_on_turn_start(void){ for(int i=0;i<selected_count;++i){ 
    Skill*s=selected_skills[i].skill; 
    if(s->on_turn_start) s->on_turn_start(s);
} 
}
void trigger_on_turn_end(void) {
    for (int i = 0; i < selected_count; i++) {
        Skill *s = selected_skills[i].skill;
        if (s->on_turn_end) s->on_turn_end(s);
    }
}


// 檢查技能有沒有滿等
static bool is_skill_maxed(Skill *skill){ 
    for(int i=0;i<selected_count;++i) 
    if(selected_skills[i].skill==skill&&selected_skills[i].level>=MAX_SKILL_LEVEL) 
    return true; 
    return false; 
}

// 技能選擇畫面
#define BOX_W 150
#define BOX_H 450
#define BOX_GAP 40
#define CORNER 20
void draw_skill_scene(void)
{
    al_draw_filled_rectangle(0,0,WIDTH,HEIGHT, al_map_rgba(50,50,50,180));
    float total = 3*BOX_W + 2*BOX_GAP;
    float sx = WIDTH/2 - total/2;
    float sy = HEIGHT/2 - BOX_H/2;

    for(int i=0;i<3;++i){
        Skill* sk = displayed_skills[i];
        bool maxed = sk && is_skill_maxed(sk);
        ALLEGRO_COLOR col = sk&&!maxed? sk->color : al_map_rgb(120,120,120);
        const char* lbl = sk&&!maxed? sk->name : "X";
        float x=sx+i*(BOX_W+BOX_GAP), y=sy; float x2=x+BOX_W, y2=y+BOX_H;
        al_draw_filled_rectangle(x+CORNER,y,x2-CORNER,y2,col);
        al_draw_filled_rectangle(x,y+CORNER,x2,y2-CORNER,col);
        al_draw_filled_circle(x+CORNER,y+CORNER,CORNER,col);
        al_draw_filled_circle(x2-CORNER,y+CORNER,CORNER,col);
        al_draw_filled_circle(x+CORNER,y2-CORNER,CORNER,col);
        al_draw_filled_circle(x2-CORNER,y2-CORNER,CORNER,col);
        al_draw_text(skill_font, al_map_rgb(0,0,0), x+BOX_W/2, y+BOX_H/2, ALLEGRO_ALIGN_CENTER, lbl);
    }
}


void update_skill_selection(void)
{
    Skill *candidates[SKL_COUNT];
    int cand_n = 0;

    // 補血技能永遠可以出現
    if (all_skills[SKL_FRUIT_HEAL].available)
        candidates[cand_n++] = &all_skills[SKL_FRUIT_HEAL];

    if (selected_count < MAX_SELECTED_SKILLS) {
        for (int i = 0; i < SKL_COUNT; ++i) {
            if (i == SKL_FRUIT_HEAL) continue;
            if (!all_skills[i].available) continue; // 只選目前開放的技能
            if (!is_skill_maxed(&all_skills[i]))
                candidates[cand_n++] = &all_skills[i];
        }
    } else {
        for (int i = 0; i < selected_count; ++i)
            if (selected_skills[i].level < MAX_SKILL_LEVEL &&
                selected_skills[i].skill->available)         // 只選目前開放的技能
                candidates[cand_n++] = selected_skills[i].skill;
    }

    // 打亂候選技能
    for (int i = cand_n-1; i>0; --i) {
        int j = rand() % (i+1);
        Skill* tmp = candidates[i];
        candidates[i]=candidates[j];
        candidates[j]=tmp;
    }

    for (int i = 0; i < 3; ++i)
        displayed_skills[i] = (i < cand_n) ? candidates[i] : NULL;
}

bool select_skill(int idx)
{
    if (idx<0 || idx>=3 || !displayed_skills[idx]) return false;
    Skill *sel = displayed_skills[idx];

    // 補血技能不佔格也不限次數
    if (sel->id == SKL_FRUIT_HEAL) { sp_fruit_heal(sel); return true; }

    for (int i = 0; i < selected_count; ++i) if (selected_skills[i].skill == sel) {
        if (selected_skills[i].level < MAX_SKILL_LEVEL) { ++selected_skills[i].level; return true; }
        return false; // 已滿等
    }

    if (selected_count < MAX_SELECTED_SKILLS) {
        selected_skills[selected_count].skill = sel;
        selected_skills[selected_count].level = 1;
        ++selected_count;
        return true;
    }
    return false;
}

void toggle_skill_scene(void){ 
    show_skill_scene = !show_skill_scene;
    game_paused      = show_skill_scene;    // 開技能畫面就暫停，關掉再繼續
    if (show_skill_scene) {
        update_skill_selection();
    }
 }
bool is_skill_scene_visible(void){ return show_skill_scene; }




void draw_skill_buttons(void)
{
    const float R = 30.0f;       // 圓的半徑
    const float GAP = 40.0f;     // 圓和圓的距離
    const float UP_SHIFT = 80.0f; // 往上移一點，別蓋到棋盤
    const float OUT = 3.0f;      // 外框粗細

    float bx, by;
    get_hex_center(10, 0, &bx, &by); // 取左下角那格當基準
    float cy = by + UP_SHIFT;        // 往上移一點，看起來比較浮

    for (int i = 0; i < MAX_SELECTED_SKILLS; ++i) {
        float cx = bx + i * (2 * R + GAP); // 從左往右排列
        ALLEGRO_COLOR col = (i < selected_count)
            ? selected_skills[i].skill->color
            : al_map_rgb(180, 250, 200);

        // 先畫按鈕
        al_draw_filled_circle(cx, cy, R, col);
        al_draw_circle(cx, cy, R, al_map_rgb(0, 100, 80), OUT);

        if (i < selected_count) {
            // 顯示等級
            char lvl[16];
            snprintf(lvl, sizeof(lvl), "Lv.%d", selected_skills[i].level);
            al_draw_text(skill_font, al_map_rgb(0, 0, 0),
                         cx, cy,
                         ALLEGRO_ALIGN_CENTER, lvl);

            // 技能名上下交錯排，避免擠在一起
            const char* name = selected_skills[i].skill->name;
            if ((i % 2) == 0) {
                al_draw_text(skill_font, al_map_rgb(0, 0, 0),
                             cx, cy - R - 20,
                             ALLEGRO_ALIGN_CENTER, name);
            } else {
                al_draw_text(skill_font, al_map_rgb(0, 0, 0),
                             cx, cy + R + 5,
                             ALLEGRO_ALIGN_CENTER, name);
            }
        }
    }
}



// 處理三個技能選項的點擊
void handle_skill_click(float mx, float my)
{
   

    float total_w = 3*BOX_W + 2*BOX_GAP;
    float sx = WIDTH/2 - total_w/2;
    float sy = HEIGHT/2 - BOX_H/2;

    for (int i = 0; i < 3; ++i) {
        float x  = sx + i*(BOX_W+BOX_GAP);
        float y  = sy;
        float x2 = x + BOX_W;
        float y2 = y + BOX_H;
        if (mx>=x && mx<=x2 && my>=y && my<=y2 && displayed_skills[i]) {
            bool ok = select_skill(i);
            printf("Skill %s %s", displayed_skills[i]->name, ok?"selected":"failed");
            toggle_skill_scene();
            break;
        }
    }
}

