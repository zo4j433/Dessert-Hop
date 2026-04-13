#include "star_explosion.h"
#include "../board/hexboard.h"
#include "../global.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include <stdlib.h>

#define MAX_STARS  32
#define STAR_LIFETIME 40
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif


typedef struct {
    float x, y;      // 當前螢幕座標
    float vx, vy;    // 速度
    int   ttl;       // 剩餘幀數
} Star;

static Star stars[MAX_STARS];
static ALLEGRO_BITMAP *star_img = NULL;

// 棋盤 (r,c) → 螢幕座標
static void hex_to_screen(int r, int c, float *sx, float *sy) {
    float W = get_hex_w(), V = get_hex_v();
    float x_center = rowOffsetTable[5]*W + 5*W + (5%2)*(W/2);
    float y_center = 5*V;
    float ox = WIDTH/2  - x_center;
    float oy = HEIGHT/2 - y_center;
    *sx = ox + rowOffsetTable[r]*W + c*W + (r%2)*(W/2);
    *sy = oy + r*V;
}

void init_star_explosion(void) {
    star_img = al_load_bitmap("assets/image/star.png");
    for (int i = 0; i < MAX_STARS; i++) stars[i].ttl = 0;
}

void spawn_star_explosion(int r, int c) {
    float cx, cy;
    hex_to_screen(r, c, &cx, &cy);
    for (int i = 0; i < MAX_STARS; i++) {
        if (stars[i].ttl <= 0) {
            // 隨機方向與速度
            float angle = ((float)rand() / RAND_MAX) * 2 * M_PI;
            float speed = 2.0f + ((float)rand() / RAND_MAX) * 2.0f;
            stars[i].x   = cx;
            stars[i].y   = cy;
            stars[i].vx  = cosf(angle) * speed;
            stars[i].vy  = sinf(angle) * speed - 1.0f; // 少許向上力道
            stars[i].ttl = STAR_LIFETIME;
        }
    }
}

void update_star_explosion(void) {
    for (int i = 0; i < MAX_STARS; i++) {
        if (stars[i].ttl > 0) {
            stars[i].x += stars[i].vx;
            stars[i].y += stars[i].vy;
            // 重力微弱下墜
            stars[i].vy += 0.1f;
            stars[i].ttl--;
        }
    }
}

void draw_star_explosion(void) {
    int tex_w = star_img ? al_get_bitmap_width(star_img) : 0;
    int tex_h = star_img ? al_get_bitmap_height(star_img) : 0;
    for (int i = 0; i < MAX_STARS; i++) {
        if (stars[i].ttl > 0) {
            float t = (float)stars[i].ttl / STAR_LIFETIME; // 1→0
            // 從 50%→100% 大小漸變
            float scale = 0.5f + 0.5f * (1 - t);
            // 漸淡
            ALLEGRO_COLOR col = al_map_rgba_f(1,1,1, t);

            if (star_img) {
                al_draw_tinted_scaled_bitmap(
                    star_img, col,
                    0,0,tex_w,tex_h,
                    stars[i].x - tex_w*scale/2,
                    stars[i].y - tex_h*scale/2,
                    tex_w*scale, tex_h*scale,
                    0
                );
            } else {
                // fallback：畫簡單五角星（用圓點表示）
                al_draw_filled_circle(
                    stars[i].x, stars[i].y, 4 * scale, col
                );
            }
        }
    }
}
