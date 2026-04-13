#include "peanut_bomb.h"
#include "../global.h"
#include "../board/hexboard.h"
#include "../ui/flash.h"            // 落地時閃光
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static PeanutBomb bombs[MAX_PEANUT_BOMBS];

// 棋盤(r,c)轉螢幕座標
static void hex_to_screen(int r, int c, float *x, float *y) {
    float W = get_hex_w(), V = get_hex_v();
    float cx = rowOffsetTable[5]*W + 5*W + (5%2)*(W/2);
    float cy = 5*V;
    float ox = WIDTH/2 - cx;
    float oy = HEIGHT/2 - cy;
    *x = ox + rowOffsetTable[r]*W + c*W + (r%2)*(W/2);
    *y = oy + r*V;
}

void init_peanut_bombs(void) {
    for (int i = 0; i < MAX_PEANUT_BOMBS; i++)
        bombs[i].active = false;
}

void spawn_peanut_bomb(int centre_r, int centre_c) {
    float sx, sy, tx, ty;
    // 中心格螢幕座標
    hex_to_screen(centre_r, centre_c, &sx, &sy);
    // 目標也設定在同一格
    hex_to_screen(centre_r, centre_c, &tx, &ty);

    // 1. 投擲一顆炸彈到中心格
    for (int i = 0; i < MAX_PEANUT_BOMBS; i++) {
        if (!bombs[i].active) {
            bombs[i].active = true;
            bombs[i].x      = sx;
            bombs[i].y      = sy;
            bombs[i].ttl    = 20;  // 30 幀後落地
            bombs[i].vx     = 0;   // 水平不動
            // 垂直：同樣使用簡易拋物線公式
            bombs[i].vy     = (ty - sy + 0.5f * 9.8f * (bombs[i].ttl/60.0f) * (bombs[i].ttl/60.0f))
                               / bombs[i].ttl;
            bombs[i].tr     = centre_r;
            bombs[i].tc     = centre_c;
            break;
        }
    }

    
}

void update_peanut_bombs(void) {
    for (int i = 0; i < MAX_PEANUT_BOMBS; i++) {
        if (!bombs[i].active) continue;
        bombs[i].ttl--;
        if (bombs[i].ttl <= 0) {
            // 2. 對六個相鄰格觸發閃光 (flash)
   int cr = bombs[i].tr;
            int cc = bombs[i].tc;
            for (int d = 0; d < 6; d++) {
                int nr, nc;
                get_neighbor(cr, cc, d, &nr, &nc);
                if (nr >= 0 && nr < ROWS &&
                    nc >= 0 && nc < rowLengths[nr]) {
                    add_flash_tile(nr, nc);
                }
            }
            add_flash_tile(bombs[i].tr, bombs[i].tc);
            bombs[i].active = false;
        } else {
            // 運動學：簡易拋物線（vy 向下加重力）
            bombs[i].x += bombs[i].vx;
            bombs[i].y += bombs[i].vy;
            bombs[i].vy += 9.8f * (1.0f/60.0f);  // 一帧重力
        }
    }
}

void draw_peanut_bombs(void) {
    for (int i = 0; i < MAX_PEANUT_BOMBS; i++) {
        if (!bombs[i].active) continue;
        // 用一個小圓代表花生炸彈，也可替換成貼圖
        al_draw_filled_circle(
            bombs[i].x, bombs[i].y,
            HEX_SIZE * 0.3f,
            al_map_rgba(200,150, 80, 255)
        );
        al_draw_circle(
            bombs[i].x, bombs[i].y,
            HEX_SIZE * 0.3f,
            al_map_rgba(100, 50, 20, 255),
            2
        );
    }
}
