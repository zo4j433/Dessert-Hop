#include "../GAME_ASSERT.h"
#include "startscene.h"
#include <allegro5/allegro_primitives.h>  // 繪製矩形
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>


static ALLEGRO_BITMAP *start_bg = NULL;


typedef struct {
    int x, y, w, h;
    const char *label;
} Button;


static Button btnStart  = { 300, 200, 200,  50, "start" };
static Button btnHelp   = { 300, 300, 200,  50, "instruction" };
static int sliderX = 300, sliderY = 400, sliderW = 200, sliderH = 10;
static float volumeLevel = 1.0f;

static ALLEGRO_FONT *font = NULL;

//字型
static void StartScene_Init(Scene *self) {
    btnStart.w = 200; btnStart.h = 50;

btnStart.x = (WIDTH - btnStart.w) / 2;
btnHelp.x  = (WIDTH - btnHelp.w)  / 2;

btnStart.y = HEIGHT/2;      
btnHelp.y  = btnStart.y + btnStart.h + 20; 

     // 載入背景圖
    if (!start_bg) {
        start_bg = al_load_bitmap("assets/image/start_bg.jpg");
        if (!start_bg) {
            // 如果載入失敗，可做 fallback 或只用純色背景
            fprintf(stderr, "Warning: 無法載入 start 背景圖 ../assets/image/start_bg.png，將使用純色背景。\n");
            // 不做 GAME_ASSERT，以免直接中斷
        }
    }
    if (!font) {
        font = al_load_ttf_font("assets/font/pirulen.ttf", 24, 0);
        if (!font) {
            fprintf(stderr, "Warning: 無法載入字型，改用內建字型。\n");
            font = al_create_builtin_font();
        }
    }
    self->scene_end = false;
}

// 偵測滑鼠是否在按鈕範圍
static bool is_inside(int mx, int my, Button *b) {
    return mx >= b->x && mx <= b->x + b->w
        && my >= b->y && my <= b->y + b->h;
}

static void StartScene_Update(Scene *self) {
    // 按下 ENTER 仍可從鍵盤啟動
    if (key_state[ALLEGRO_KEY_ENTER]) {
        key_state[ALLEGRO_KEY_ENTER] = false;
        self->scene_end = true;  // 切到下一場景 (MenuScene)
    }

    // 滑鼠點擊
    if (mouse_state[1]) {
        mouse_state[1] = false;  // 只處理一次
        int mx = mouse.x, my = mouse.y;

        // 開始遊戲
        if (is_inside(mx, my, &btnStart)) {
            self->scene_end = true;  // 由外層切到 Menu 或 直接 GameScene
        }
        // 遊戲說明
        else if (is_inside(mx, my, &btnHelp)) {
             mouse_state[1] = false;
        scene->Destroy(scene);
        window = Instruction_L;
        create_scene(Instruction_L);
        return;
        }
        // 音量滑桿範圍內就改音量
        else if (mx >= sliderX && mx <= sliderX + sliderW
              && my >= sliderY - 5 && my <= sliderY + sliderH + 5)
        {
            volumeLevel = (float)(mx - sliderX) / sliderW;
            if (volumeLevel < 0) volumeLevel = 0;
            if (volumeLevel > 1) volumeLevel = 1;
            // 實際設定全域音量
            al_set_mixer_gain(NULL, volumeLevel);
        }
    }
}

static void StartScene_Draw(Scene *self) {
    // 背景
    al_clear_to_color(al_map_rgb(30, 30, 60));
    if (start_bg) {
        int bw = al_get_bitmap_width(start_bg);
        int bh = al_get_bitmap_height(start_bg);
        al_draw_scaled_bitmap(start_bg, 0,0,bw,bh, 0,0,WIDTH,HEIGHT, 0);
    } else {
        al_clear_to_color(al_map_rgb(30, 30, 60));
    }

    //按鈕
    Button *buttons[2] = { &btnStart, &btnHelp };
    for (int i = 0; i < 2; i++) {
        Button *b = buttons[i];
    
        // 文字
        al_draw_text(font, al_map_rgb(255, 255, 255),
                     b->x + b->w/2, b->y + b->h/2 - 12,
                     ALLEGRO_ALIGN_CENTRE, b->label);
    }

  
}

static void StartScene_Destroy(Scene *self) {
    if (font) {
        al_destroy_font(font);
        font = NULL;
    }
    free(self);
}

Scene* New_StartScene(SceneType type) {
    Scene *s = New_Scene(type);
    StartScene_Init(s);
    s->Update  = StartScene_Update;
    s->Draw    = StartScene_Draw;
    s->Destroy = StartScene_Destroy;
    return s;
}
