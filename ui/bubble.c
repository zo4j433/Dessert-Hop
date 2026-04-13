// bubble.c
#include "bubble.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "../global.h"

#define MAX_BUBBLES 64
static MintBubble bubbles[MAX_BUBBLES];
static ALLEGRO_BITMAP *bubble_img = NULL;

void init_bubbles(void) {
    bubble_img = al_load_bitmap("assets/image/mint_bubble.png");
    for (int i = 0; i < MAX_BUBBLES; i++) bubbles[i].ttl = 0;
}

static void hex_to_screen(int r, int c, float *x, float *y) {
    float W = get_hex_w(), V = get_hex_v();
    float x_center = rowOffsetTable[5]*W + 5*W + (5%2)*(W/2);
    float y_center = 5*V;
    float ox = WIDTH/2  - x_center;
    float oy = HEIGHT/2 - y_center;
    *x = ox + rowOffsetTable[r]*W + c*W + (r%2)*(W/2);
    *y = oy + r*V;
}

void spawn_mint_bubble(int from_r, int from_c, int to_r, int to_c) {
    // 找空位
    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (bubbles[i].ttl <= 0) {
            float sx, sy, tx, ty;
            hex_to_screen(from_r, from_c, &sx, &sy);
            hex_to_screen(to_r,   to_c,   &tx, &ty);
            // 往怪物頭頂偏移一點
            ty -= get_hex_h()*0.4f;
            bubbles[i].x   = sx;
            bubbles[i].y   = sy;
            bubbles[i].tx  = tx;
            bubbles[i].ty  = ty;
            bubbles[i].ttl = 30;  // 動畫 30 幀
            break;
        }
    }
}

void update_bubbles(void) {
    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (bubbles[i].ttl > 0) {
            float t = (float)bubbles[i].ttl / 30.0f;
            // 線性插值
            bubbles[i].x = bubbles[i].tx + (bubbles[i].x - bubbles[i].tx) * t;
            bubbles[i].y = bubbles[i].ty + (bubbles[i].y - bubbles[i].ty) * t;
            bubbles[i].ttl--;
        }
    }
}

void draw_bubbles(void) {
    float hexW = get_hex_h();
    float hexH = get_hex_h();

    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (bubbles[i].ttl > 0 && bubble_img) {
            // 1. 計算剩餘動畫百分比 t：從 1→0
            float t = (float)bubbles[i].ttl / 30.0f;

            // 2. 線性插值 scaleFactor：從 0.2 → 1.0
            float scaleFactor = 0.2f + 0.5f * (1.0f - t);

            // 3. 計算實際畫的寬高（不超過一格）
            float drawW = hexW * scaleFactor;
            float drawH = hexH * scaleFactor;

            // 4. 畫圖，中心對齊到 bubbles[i].x/y
            int texW = al_get_bitmap_width(bubble_img);
            int texH = al_get_bitmap_height(bubble_img);
            al_draw_scaled_bitmap(
                bubble_img,
                0, 0, texW, texH,
                bubbles[i].x - drawW/2,
                bubbles[i].y - drawH/2,
                drawW, drawH,
                0
            );
        }
    }
}
