#include "../GAME_ASSERT.h"
#include "instructionscene.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>



static ALLEGRO_BITMAP *instr_bg = NULL;
static ALLEGRO_FONT   *font_ins = NULL;

static float panel_x, panel_y, panel_w, panel_h;
static float btn_x, btn_y, btn_w, btn_h;
static float padding;  

// 載入背景與字型，並計算面板與按鈕位置
static void Instruction_Init(Scene *self) {
    fprintf(stderr, "[InstructionScene] Init\n");
    //載入背景圖
    if (!instr_bg) {
        instr_bg = al_load_bitmap("assets/image/background.png");
        if (!instr_bg) {
            fprintf(stderr, "[InstructionScene] 載入背景失敗: ../assets/image/instruction_bg.png，將使用純色背景\n");
        } else {
            fprintf(stderr, "[InstructionScene] 背景載入成功\n");
        }
    }
    //載入字型
    if (!font_ins) {
         font_ins = al_create_builtin_font();
        if (!font_ins) {
            fprintf(stderr, "[InstructionScene] 載入字型失敗，使用內建字型\n");
            font_ins = al_create_builtin_font();
        } else {
            fprintf(stderr, "[InstructionScene] 字型載入成功\n");
        }
    }
    
    panel_w = WIDTH * 0.8f;
    panel_h = HEIGHT * 0.6f;
    panel_x = (WIDTH - panel_w) / 2.0f;
    panel_y = (HEIGHT - panel_h) / 2.0f + 20.0f;  

    padding = 20.0f;
    btn_w = 120.0f;
    btn_h = 40.0f;
    btn_x = panel_x + (panel_w - btn_w) / 2.0f;
    btn_y = panel_y + panel_h - btn_h - padding;
    if (self) {
        self->scene_end = false;
    }
}

// Update: 偵測返回按鈕點擊或按鍵 B/b
static void Instruction_Update(Scene *self) {
    // 偵測滑鼠點擊
    if (mouse_state[1]) {
        int mx = mouse.x;
        int my = mouse.y;
        mouse_state[1] = false;
        // 判斷是否點到返回按鈕
        if (mx >= btn_x && mx <= btn_x + btn_w
            && my >= btn_y && my <= btn_y + btn_h) {
            self->scene_end = true;
        }
    }
    // 偵測鍵盤 B 或 b
    if (key_state['B'] || key_state['b']) {
        key_state['B'] = key_state['b'] = false;
        self->scene_end = true;
    }
}

static void Instruction_Draw(Scene *self) {
    //背景
    if (instr_bg) {
        int bw = al_get_bitmap_width(instr_bg);
        int bh = al_get_bitmap_height(instr_bg);
        al_draw_scaled_bitmap(
            instr_bg,
            0, 0, bw, bh,
            0, 0, WIDTH, HEIGHT,
            0
        );
    } else {
        // fallback 純色
        al_clear_to_color(al_map_rgb(20, 20, 20));
    }

    ALLEGRO_COLOR panel_color = al_map_rgba(0, 0, 0, 160);    // alpha 160
    float radius = 20.0f;
    al_draw_filled_rounded_rectangle(
        panel_x, panel_y,
        panel_x + panel_w, panel_y + panel_h,
        radius, radius,
        panel_color
    );
    // 邊框
    ALLEGRO_COLOR border_color = al_map_rgba(255, 255, 255, 200);
    al_draw_rounded_rectangle(
        panel_x, panel_y,
        panel_x + panel_w, panel_y + panel_h,
        radius, radius,
        border_color,
        2.0f
    );
    //說明文字
    const char *text =
    "Dessert Hop! Quick Introduction:\n"
    "\n"
    "Welcome to Dessert Hop!, a turn-based hopping adventure where you plan jump paths, clear sweet-themed monsters, and survive through 50 turns to win.\n"
    "\n"
    "How to Play:\n"
    "- Plan Your Jump: Click highlighted tiles to build a hop sequence (yellow for empty hops; green to attack monsters along the path).\n"
    "- Execute Movement: Press Enter to auto-hop along your path. Jumping over a monster deals damage; defeating enemies can trigger combos and skills.\n"
    "- Enemy Phase: After your move, surviving monsters move toward you and attack. Keep an eye on your HP!\n"
    "- Skills & Upgrades: Defeat enemies to earn experience and level up. Choose from random skills to upgrade (e.g., Jelly Burst, Peanut Bombs, Sugar Dust Trail).\n"
    "- Victory & Defeat: Survive through turn 50 to win; if your HP drops to zero at any time, it’s game over.\n"
    "\n"
    "Good luck, and enjoy hopping through the dessert battlefield!\n"
     "\n"
    "Skill List:\n"
    "1. Jelly Burst: When landing on a junction tile, explode jelly to deal Level x 25% damage to enemies along your path.\n"
    "2. Mint Bubble Puff: At each landing, drop a bubble that deals Level x 24% damage to all enemies within 1 tile.\n"
    "3. Cream Filled Cookie: On the final landing, trigger a cookie blast that deals Level x 50% damage to enemies within 2 tiles.\n"
    "4. Cream Core: When you kill an enemy, deal Level x 45% damage to all enemies within 1 tile of the defeated target.\n"
    "5. Peanut Bombs: At the start of your turn, drop peanut bombs on up to 3 monsters, each dealing Level x 30% damage in radius 1.\n"
    "6. Caramel Bombs: At the start of your turn, drop bombs at 3 random positions, each dealing Level x 24% damage in radius 1.\n"
    "7. Sugar Dust Trail: Leave sugar dust on each hopped tile; each dust tile deals Level x 24% damage when active.\n"
    "8. Sugar Rush: When your HP is below 50%, increase your attack power by Level x 20%.\n"
    "9. Snow Frosting: Permanently increase your attack power by Level x 6%.\n"
    "10. Cream Lifesteal: Heal for Level x 4% of the damage you deal.\n"
    "11. Sweet Counter: When you are attacked, counterattack and deal Level x 50% damage back to the attacker.\n"
    "12. Cream Shield: Reduce all damage taken by Level x 4%.\n"
    "13. Fruit Medica: Restore half of your maximum HP. This skill does not occupy a skill slot and has no upgrade limit.\n"
    "\n"
    "(All skills above except #15 can be leveled from 1 to 6. Obtaining the same skill again increases its level.)\n";

    //右對齊
    float text_max_w = panel_w - 2 * padding;
    float text_y = panel_y + padding;
    float text_x = panel_x + padding;
    float line_h = al_get_font_line_height(font_ins);
    // 用置中對齊
    al_draw_multiline_text(
        font_ins,
        al_map_rgb(230, 230, 230),
        text_x,
        text_y,
        text_max_w,
        line_h,
        ALLEGRO_ALIGN_LEFT,
        text
    );
    //返回按鈕
   
   
    // 按鈕文字垂直置中計算
    float text_h = al_get_font_line_height(font_ins);
    al_draw_text(
        font_ins,
        al_map_rgb(255, 255, 255),
        btn_x + btn_w / 2.0f,
        btn_y + (btn_h - text_h) / 2.0f,
        ALLEGRO_ALIGN_CENTRE,
        "back"
    );
    // al_flip_display() 由外層 game_draw 呼
}

static void Instruction_Destroy(Scene *self) {
    if (instr_bg) {
        al_destroy_bitmap(instr_bg);
        instr_bg = NULL;
    }
    if (font_ins) {
        al_destroy_font(font_ins);
        font_ins = NULL;
    }
    free(self);
}

// Factory
Scene* New_InstructionScene(SceneType type) {
    Scene *s = New_Scene(type);
    Instruction_Init(s);
    s->scene_end = false;
    s->Update    = Instruction_Update;
    s->Draw      = Instruction_Draw;
    s->Destroy   = Instruction_Destroy;
    return s;
}
