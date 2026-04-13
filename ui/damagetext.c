#include "DamageText.h"
#include "../board/hexboard.h"
#include "../global.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

static ALLEGRO_FONT *dmg_font = NULL;
static DamageText dmg_texts[MAX_DAMAGE_TEXT];

// 每條文字的生命週期
static const int LIFE_SPAN = 30;
static const int DELAY_STEP = 5;

void show_damage_text(int r, int c, int damage, DamageTarget target) {
    // 找到一個空 slot
    for (int i = 0; i < MAX_DAMAGE_TEXT; i++) {
        if (dmg_texts[i].frames_left <= 0 && dmg_texts[i].delay <= 0) {
            // 先統計目前同類文字還在排隊的數量
            int same = 0;
            for (int j = 0; j < MAX_DAMAGE_TEXT; j++) {
                if (dmg_texts[j].target == target && 
                    (dmg_texts[j].delay > 0 || dmg_texts[j].frames_left > 0)) {
                    same++;
                }
            }
            dmg_texts[i].r           = r;
            dmg_texts[i].c           = c;
            dmg_texts[i].damage      = damage;
            dmg_texts[i].target      = target;
            dmg_texts[i].delay       = same * DELAY_STEP;
            dmg_texts[i].frames_left = LIFE_SPAN;
            return;
        }
    }
}

void draw_damage_texts(void) {
    if (!dmg_font) 
        dmg_font = al_create_builtin_font();

    float HEX_W = get_hex_w(), HEX_V = get_hex_v();
    // 計算棋盤畫面偏移（同 draw_board）
    float x_center = rowOffsetTable[5] * HEX_W + 5 * HEX_W + (5 % 2) * (HEX_W/2);
    float y_center = 5 * HEX_V;
    float ox = WIDTH/2 - x_center;
    float oy = HEIGHT/2 - y_center;

    for (int i = 0; i < MAX_DAMAGE_TEXT; i++) {
        DamageText *dt = &dmg_texts[i];

        // 先處理延遲
        if (dt->delay > 0) {
            dt->delay--;
            continue;
        }
        if (dt->frames_left <= 0) 
            continue;

        // 計算要畫的位置
        float x, y;
        if (dt->target == TARGET_MONSTER) {
            int r = dt->r, c = dt->c;
            float row_off = rowOffsetTable[r] * HEX_W;
            x = ox + row_off + c * HEX_W + (r % 2) * (HEX_W/2);
            // 往上漂浮：利用已消耗的幀數
            y = oy + r * HEX_V - (LIFE_SPAN - dt->frames_left);
        } else {
            // 玩家文字從左下方彈出
            x = 20;
            y = HEIGHT - 40 - (LIFE_SPAN - dt->frames_left);
        }

        // 顏色與格式
        ALLEGRO_COLOR color = (dt->target == TARGET_MONSTER)
                                ? al_map_rgb(255, 50, 50)
                                : al_map_rgb(255,255,50);
        const char *fmt = (dt->target == TARGET_MONSTER) ? "-%d" : "+%d";

        al_draw_textf(dmg_font, color, x, y, ALLEGRO_ALIGN_CENTRE,
                      fmt, dt->damage);

        dt->frames_left--;
    }
}
