#include "flash.h"
#include "../board/hexboard.h"
#include "../global.h"
#include <allegro5/allegro_primitives.h>

static FlashTile tiles[MAX_FLASH];
static int flash_cnt = 0;

void flash_init(void) {
    flash_cnt = 0;
}

void add_flash_tile(int r, int c) {
    for (int i = 0; i < flash_cnt; i++) {
        if (tiles[i].r == r && tiles[i].c == c) {
            tiles[i].frames_left = 6;
            return;
        }
    }
    if (flash_cnt < MAX_FLASH) {
        tiles[flash_cnt++] = (FlashTile){r, c, 6};
    }
}

void flash_update(void) {
    for (int i = 0; i < flash_cnt; ) {
        if (--tiles[i].frames_left <= 0) {
            tiles[i] = tiles[--flash_cnt];
        } else {
            i++;
        }
    }
}

void flash_draw(void) {
    float W = get_hex_w();
    float V = get_hex_v();
    float x_center = rowOffsetTable[5] * W + 5 * W + (5 % 2) * (W / 2);
    float y_center = 5 * V;
    float ox = WIDTH / 2.0f - x_center;
    float oy = HEIGHT / 2.0f - y_center;

    float radius = W * 0.4f;

    for (int i = 0; i < flash_cnt; i++) {
        int r = tiles[i].r, c = tiles[i].c;
        float cx = ox
                 + rowOffsetTable[r] * W
                 + c * W
                 + (r % 2) * (W / 2);
        float cy = oy + r * V;
        al_draw_filled_circle(
            cx, cy,
            radius,
            al_map_rgba(255, 255, 120, 180)
        );
    }
}
