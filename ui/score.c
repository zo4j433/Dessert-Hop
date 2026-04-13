#include "ScoreText.h"
#include "../global.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

typedef struct {
    int   value;     // +分數
    int   frames;    // 還剩幾幀
    int   delay;     // 延遲幀數
} ScoreText;

static ScoreText st[MAX_SCORE_TEXT];
static ALLEGRO_FONT *font = NULL;

static const int LIFE = 60;      // 顯示 1 秒
static const int STEP = 5;       // 每條錯開 5 幀

void show_score_text(int pts) {
    // 找空槽
    for (int i = 0; i < MAX_SCORE_TEXT; i++) {
        if (st[i].frames <= 0 && st[i].delay <= 0) {
            // 計算目前還在排隊的
            int same = 0;
            for (int j = 0; j < MAX_SCORE_TEXT; j++)
                if (st[j].delay > 0 || st[j].frames > 0)
                    same++;
            st[i].value = pts;
            st[i].delay = same * STEP;
            st[i].frames = LIFE;
            return;
        }
    }
}

void draw_score_texts(void) {
    if (!font)
        font = al_create_builtin_font();

    // 文字基準點：右上，跟 Score UI 對齊
    int txt_h = al_get_font_line_height(font);
    float base_x = WIDTH - 10;
    float base_y = 10 + txt_h + 8;  

    for (int i = 0; i < MAX_SCORE_TEXT; i++) {
        ScoreText *s = &st[i];
        // delay
        if (s->delay > 0) {
            s->delay--;
            continue;
        }
        if (s->frames <= 0)
            continue;

        // 漂浮效果：每幀往上 1px
        float y = base_y - (LIFE - s->frames);
        // 透明度：後半段開始淡出
        float a = s->frames < (LIFE/2) 
                ? (s->frames / (float)(LIFE/2)) 
                : 1.0f;
        ALLEGRO_COLOR col = al_map_rgba(200,255,200, (int)(255*a));

        char buf[16];
        snprintf(buf, sizeof(buf), "+%d", s->value);

        al_draw_text(font, col, base_x, y, ALLEGRO_ALIGN_RIGHT, buf);
        s->frames--;
    }
}
