#include "healtext.h"
#include "../global.h"
#include "../board/hexboard.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

typedef struct {
    bool   active;
    int    r, c;
    int    amount;
    float  y_offset;  // 从格子中心往上漂移
    int    ttl;
} HealText;

#define MAX_HEAL_TEXT  16

static HealText heals[MAX_HEAL_TEXT];
static ALLEGRO_FONT *heal_font = NULL;

// 在游戏初始化时调用一次
void init_heal_text(void) {
    for (int i = 0; i < MAX_HEAL_TEXT; i++) heals[i].active = false;
    if (!heal_font) {
        heal_font = al_create_builtin_font();
    }
}

// r,c：棋盘坐标，amount：回血量
void show_heal_text(int r, int c, int amount) {
    for (int i = 0; i < MAX_HEAL_TEXT; i++) {
        if (!heals[i].active) {
            heals[i].active   = true;
            heals[i].r        = r;
            heals[i].c        = c;
            heals[i].amount   = amount;
            heals[i].y_offset = 0;
            heals[i].ttl      = 60;  // 持续 60 帧（1 秒）
            break;
        }
    }
}

// 每帧更新：在主循环 update 阶段调用
void update_heal_text(void) {
    for (int i = 0; i < MAX_HEAL_TEXT; i++) {
        if (!heals[i].active) continue;
        heals[i].ttl--;
        heals[i].y_offset -= 0.5f;  // 向上漂移
        if (heals[i].ttl <= 0) {
            heals[i].active = false;
        }
    }
}

// 每帧绘制：放在 draw_player() 或 draw_monsters() 之后
void draw_heal_text(void) {
    float W = get_hex_w(), V = get_hex_v();
    for (int i = 0; i < MAX_HEAL_TEXT; i++) {
        if (!heals[i].active) continue;
        // 计算格子屏幕中心
        float row_off = rowOffsetTable[heals[i].r]*W + (heals[i].r%2)*(W/2);
        float cx = WIDTH/2 - (rowOffsetTable[5]*W + 5*W + (5%2)*(W/2))
                   + row_off + heals[i].c*W;
        float cy = HEIGHT/2 - (5*V)
                   + heals[i].r*V + heals[i].y_offset;
        // 绿色渐淡
        float alpha = (float)heals[i].ttl / 60.0f;
        al_draw_textf(heal_font,
                      al_map_rgba_f(0.2f,1.0f,0.2f, alpha),
                      cx, cy,
                      ALLEGRO_ALIGN_CENTER,
                      "+%d", heals[i].amount);
    }
}
