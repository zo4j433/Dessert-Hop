#include "../GAME_ASSERT.h"
#include "gameoverscene.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include "../global.h"    // 包含 extern bool exit_requested, extern bool game_won, extern int final_score


extern bool key_state[];

// Button 結構
typedef struct {
    float x, y, w, h;
    const char *label;
} Button;

static ALLEGRO_BITMAP *bg_bitmap = NULL;
static ALLEGRO_FONT   *font_go = NULL;

static Button btnExit;     // Exit 按鈕位置
// 也可保留 Return to Start 按鈕，如要移除可只留 Exit

static void GameOver_Init(Scene *self) {
    fprintf(stderr, "[GameOverScene] Init (win=%d, score=%d)\n", game_won, final_score);

    //載入字型
    if (!font_go) {
        font_go = al_load_ttf_font("assets/font/pirulen.ttf", 32, 0);
        if (!font_go) {
            fprintf(stderr, "[GameOverScene] load font failed, use builtin\n");
            font_go = al_create_builtin_font();
        }
    }
    //載入背景
    if (!bg_bitmap) {
        bg_bitmap = al_load_bitmap("assets/image/background.png");
        if (!bg_bitmap) {
            fprintf(stderr, "[GameOverScene] load bg image failed, fallback\n");
        }
    }
    //計算 Exit 按鈕位置
    btnExit.w = 200;
    btnExit.h = 50;
    btnExit.x = (WIDTH - btnExit.w) / 2.0f;
    btnExit.y = HEIGHT * 2.0f / 3.0f;
    btnExit.label = "Exit";

    // 初始化 scene_end
    if (self) self->scene_end = false;
}

static void GameOver_Update(Scene *self) {
    // 檢測滑鼠點擊 Exit
    if (mouse_state[1]) {
        int mx = mouse.x, my = mouse.y;
        mouse_state[1] = false;
        if (mx >= btnExit.x && mx <= btnExit.x + btnExit.w
         && my >= btnExit.y && my <= btnExit.y + btnExit.h) {
            // 請求退出
            exit_requested = true;
            self->scene_end = true;  // 讓外層 game_update 知道要切場景
            return;
        }
    }
    // 也可用按鍵 Esc、Q 等退出
    if (key_state[ALLEGRO_KEY_ESCAPE] || key_state['Q'] || key_state['q']) {
        key_state[ALLEGRO_KEY_ESCAPE] = false;
        key_state['Q'] = key_state['q'] = false;
        exit_requested = true;
        self->scene_end = true;
        return;
    }
}

static void GameOver_Draw(Scene *self) {
    //背景
    if (bg_bitmap) {
        int bw = al_get_bitmap_width(bg_bitmap);
        int bh = al_get_bitmap_height(bg_bitmap);
        al_draw_scaled_bitmap(bg_bitmap, 0,0,bw,bh, 0,0,WIDTH,HEIGHT, 0);
    } else {
        if (game_won) al_clear_to_color(al_map_rgb(30,100,30));
        else         al_clear_to_color(al_map_rgb(100,30,30));
    }
    //顯示勝負文字
    const char *msg = game_won ? "You Win!" : "Game Over";
    float text_w = al_get_text_width(font_go, msg);
    float text_h = al_get_font_line_height(font_go);
    al_draw_text(font_go,
                 game_won ? al_map_rgb(255,255,200) : al_map_rgb(255,200,200),
                 (WIDTH - text_w)/2.0f,
                 HEIGHT/4.0f - text_h/2.0f,
                 0,
                 msg);
    // 顯示分數
    char buf[64];
    snprintf(buf, sizeof(buf), "Score: %d", final_score);
    float sw = al_get_text_width(font_go, buf);
    al_draw_text(font_go,
                 al_map_rgb(255,255,255),
                 (WIDTH - sw)/2.0f,
                 HEIGHT/4.0f + text_h,
                 0,
                 buf);
    //繪 Exit 按鈕


    float bw_text = al_get_text_width(font_go, btnExit.label);
    float bh_text = al_get_font_line_height(font_go);
    al_draw_text(font_go,
                 al_map_rgb(255,255,255),
                 btnExit.x + (btnExit.w - bw_text)/2.0f,
                 btnExit.y + (btnExit.h - bh_text)/2.0f,
                 0,
                 btnExit.label);
    // 外層 game_draw() 最後 al_flip_display()
}

static void GameOver_Destroy(Scene *self) {
    free(self);
}

Scene* New_GameOverScene(SceneType type) {
    Scene *s = New_Scene(type);
    GameOver_Init(s);
    s->Update  = GameOver_Update;
    s->Draw    = GameOver_Draw;
    s->Destroy = GameOver_Destroy;
    return s;
}
