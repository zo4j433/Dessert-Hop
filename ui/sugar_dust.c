#include "sugar_dust.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include "../skill/skill.h"     // for selected_skills, selected_count, SKL_SUGAR_DUST
#include "../monster/monster.h" // for damage_monster_at
#include "../player/player.h"   // for player.atk, player.damage_multiplier
#include "../global.h"
#include "../board/hexboard.h"  // for find_seg_direction, get_neighbor, ROWS, rowLengths, HEX_SIZE
#include "../skill/skill.h"     // for selected_skills[], selected_count, SKL_SUGAR_DUST
#include "../ui/flash.h"        
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define MAX_SUGAR_TILES 256

// ui/sugar_dust.c

// 对应 skill.c 中的静态版本，搬过来并去掉 static
int find_seg_direction(int sr, int sc, int er, int ec) {
    for (int d = 0; d < 6; d++) {
        int r = sr, c = sc;
        while (1) {
            int nr, nc;
            get_neighbor(r, c, d, &nr, &nc);
            if (nr == er && nc == ec) return d;
            if (nr < 0 || nr >= ROWS)            break;
            if (nc < 0 || nc >= rowLengths[nr])  break;
            r = nr; c = nc;
        }
    }
    return -1;
}


// 记录本回合所有撒糖粉的格子
static Pos sugar_tiles[MAX_SUGAR_TILES];
static int sugar_cnt = 0;

void init_sugar_dust(void) {
    sugar_cnt = 0;
}

void clear_sugar_dust(void) {
    sugar_cnt = 0;
}

void add_sugar_dust_tile(int r, int c) {
     if (sugar_cnt < MAX_SUGAR_TILES) {
        sugar_tiles[sugar_cnt++] = (Pos){r, c};
        printf("[SugarDust] added at (%d,%d), total=%d\n", r, c, sugar_cnt);
    }
}

void draw_sugar_dust(void) {
    // 每个格子画一个小六边形
    float W = get_hex_w(), V = get_hex_v();
    float r_small = HEX_SIZE * 0.5f;
    ALLEGRO_COLOR col = al_map_rgba(255,200,200,160);
    for (int i = 0; i < sugar_cnt; i++) {
        int r = sugar_tiles[i].r, c = sugar_tiles[i].c;
        // hex->screen
        float xc = rowOffsetTable[5]*W + 5*W + (5%2)*(W/2);
        float yc = 5*V;
        float ox = WIDTH/2 - xc;
        float oy = HEIGHT/2 - yc;
        float cx = ox + rowOffsetTable[r]*W + c*W + (r%2)*(W/2);
        float cy = oy + r*V;

        // 顶点
        float vx[6], vy[6];
        for (int k = 0; k < 6; k++) {
            float ang = -M_PI/2 + k*(2*M_PI/6);
            vx[k] = cx + r_small * cosf(ang);
            vy[k] = cy + r_small * sinf(ang);
        }
        // 六个三角形拼六边形
        for (int k = 0; k < 6; k++) {
            int nk = (k+1)%6;
            al_draw_filled_triangle(
                cx, cy,
                vx[k], vy[k],
                vx[nk], vy[nk],
                col
            );
        }
    }
}


void apply_sugar_dust_to_monster(int r, int c) {
    // 如果 (r,c) 在 sugar_tiles 里，就造成伤害
    for (int i = 0; i < sugar_cnt; i++) {
        if (sugar_tiles[i].r == r && sugar_tiles[i].c == c) {
            // 找到 sugar skill 的 level
            int lvl = 0;
            for (int j = 0; j < selected_count; j++) {
                Skill *sk = selected_skills[j].skill;
                if (sk->id == SKL_SUGAR_DUST) {
                    lvl = selected_skills[j].level;
                    break;
                }
            }
            if (lvl > 0) {
                // 伤害 = 12% × level
                float pct = 12.0f * lvl;
                int dmg = (int)(player.atk * pct / 100.0f + 0.5f);
                // 特效
                add_flash_tile(r, c);
                damage_monster_at(r, c, dmg);
            }
            break;
        }
    }
}
