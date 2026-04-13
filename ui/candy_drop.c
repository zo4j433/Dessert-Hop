#include "candy_drop.h"
#include "../ui/flash.h"     // add_flash_tile
#include <allegro5/allegro_primitives.h>
#include "../global.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static CandyDrop drops[MAX_CANDY_DROPS];

static void hex_to_screen(int r, int c, float *x, float *y) {
    float W = get_hex_w(), V = get_hex_v();
    float cx = rowOffsetTable[5]*W + 5*W + (5%2)*(W/2);
    float cy = 5*V;
    float ox = WIDTH/2 - cx, oy = HEIGHT/2 - cy;
    *x = ox + rowOffsetTable[r]*W + c*W + (r%2)*(W/2);
    *y = oy + r*V;
}

void init_candy_drop(void) {
    for (int i = 0; i < MAX_CANDY_DROPS; i++)
        drops[i].active = false;
}

void spawn_candy_drop(int r, int c) {
    float cx, cy;
    hex_to_screen(r, c, &cx, &cy);
    float start_y = cy - HEX_SIZE * 1.5f;
    for (int i = 0; i < MAX_CANDY_DROPS; i++) {
        if (!drops[i].active) {
            drops[i].active = true;
            drops[i].x      = cx;
            drops[i].y      = start_y;
            drops[i].vy     = 0;
            drops[i].tr     = r;
            drops[i].tc     = c;
            break;
        }
    }
}

void update_candy_drop(void) {
    const float gravity = 2000.0f; // px/s²
    const float dt = 1.0f/60.0f;   
    for (int i = 0; i < MAX_CANDY_DROPS; i++) {
        if (!drops[i].active) continue;
        // 更新位置
        drops[i].vy += gravity * dt;
        drops[i].y  += drops[i].vy * dt;
       
        float tx, ty;
        hex_to_screen(drops[i].tr, drops[i].tc, &tx, &ty);
        if (drops[i].y >= ty) {
            add_flash_tile(drops[i].tr, drops[i].tc);
            drops[i].active = false;
        }
    }
}

void draw_candy_drop(void) {
    for (int i = 0; i < MAX_CANDY_DROPS; i++) {
        if (!drops[i].active) continue;
        // 白色小球
        al_draw_filled_circle(
            drops[i].x, drops[i].y,
            HEX_SIZE * 0.2f,
            al_map_rgba(255,255,255,255)
        );
        al_draw_circle(
            drops[i].x, drops[i].y,
            HEX_SIZE * 0.2f,
            al_map_rgba(200,200,200,255),
            2
        );
    }
}
