#include "turn_system.h"
#include "../monster/monster.h"
#include "../player/player.h"
#include "../board/movable.h"
#include "../global.h"
#include "../ui/status.h"   
#include "../skill/skill.h"
#include <stdio.h>
#include <allegro5/allegro_primitives.h>
ALLEGRO_FONT *status_font = NULL;
static ALLEGRO_FONT *combo_sub_font = NULL; 

int current_turn = 1;
int   combo_hits   = 0;     // 本回合跳過怪的數
float combo_bonus  = 1.0f;  // 本回合攻擊倍率
int   score        = 0;     // 總積分
int   raw_pts_this_turn = 0;
TurnState turn_state = TURN_PLAYER;
bool is_manual_jump = false;
int get_current_turn(void){
    return current_turn;
}
void monster_turn() {
    for (int i = 0; i < MAX_MONSTERS; i++) {
        if (monsters[i].alive) {
            monster_take_action(i, player.r, player.c, &player.hp);
        }
    }
}

static void start_player_turn(void) {
    reset_walked_paths(); 
    first_step   = true;
    jump_enabled = true; 
    path_len = 0;   
     if (!auto_moving)        
        update_movable_tiles();
    trigger_on_turn_start();
    turn_state = TURN_PLAYER;
}

void next_turn() {
    if (turn_state == TURN_PLAYER) {
        // 如果還在自動走 → 不進怪物回合
        if (auto_moving) return;
           //結算 combo
    if (combo_hits > 4) {
        int extra = combo_hits - 4;
        float atk_amp = 1.0f + extra * 0.04f;   // +4 % / hit
        float score_amp = 1.0f + extra * 0.02f; // +2 % / hit
        combo_bonus = atk_amp;

        // 本回合擊殺累積的 raw_pts 乘以加成後再加到總分
        score += (int)(raw_pts_this_turn * score_amp);
    } else {
        combo_bonus = 1.0f;
        score += raw_pts_this_turn;
    }

    // 重置
    combo_hits   = 0;
    raw_pts_this_turn = 0;
        //怪物回合
        turn_state = TURN_MONSTER;
       debug_print_monsters(); 
        trigger_on_turn_end();  
        monster_turn();

        spawn_monsters_if_needed(player.r, player.c);
        current_turn++;
        printf("=== Turn %d ===\n", current_turn);

        // 回到玩家回合
        start_player_turn();
    }
}
//繪製左上 Combo
void draw_combo_ui(void)
{
    
    if (!status_font)
        status_font = al_create_builtin_font();
    if (!combo_sub_font)
        combo_sub_font = al_load_ttf_font("assets/font/pirulen.ttf", 16, 0);

   
    char buf_main[32];
    snprintf(buf_main, sizeof(buf_main), "Combo x %d", combo_hits);
    int w_main = al_get_text_width(status_font, buf_main);
    int h_main = al_get_font_line_height(status_font);


    bool has_sub = (combo_hits >= 5);
    char buf_sub[64] = {0};
    int w_sub = 0, h_sub = 0;
    if (has_sub) {
        int extra     = combo_hits - 4;
        int atk_pct   = (int)((1.0f + extra * 0.04f) * 100.0f + 0.5f);
        int score_pct = (int)((1.0f + extra * 0.02f) * 100.0f + 0.5f);
        snprintf(buf_sub, sizeof(buf_sub),
                 "atk +%d%%    score +%d%%", atk_pct, score_pct);
        w_sub = al_get_text_width(combo_sub_font, buf_sub);
        h_sub = al_get_font_line_height(combo_sub_font);
    }

   
    const int PAD = 6, R = 6;
    int content_w = has_sub ? (w_main > w_sub ? w_main : w_sub) : w_main;
    int box_w     = content_w + PAD * 2;
    int box_h     = h_main + (has_sub ? (h_sub + PAD) : 0) + PAD * 2;

    float x1 = 10, y1 = 10;
    float x2 = x1 + box_w;
    float y2 = y1 + box_h;


    al_draw_filled_rounded_rectangle(x1, y1, x2, y2, R, R, al_map_rgba(0, 0, 0, 160));


    al_draw_text(status_font, al_map_rgb(255,200,0), x1 + PAD, y1 + PAD,
                 ALLEGRO_ALIGN_LEFT, buf_main);

    if (has_sub) {
        al_draw_text(combo_sub_font, al_map_rgb(200,200,200),
                     x1 + PAD,
                     y1 + PAD + h_main + PAD / 2,
                     ALLEGRO_ALIGN_LEFT, buf_sub);
    }
}


// 右上 Score 
void draw_score_ui(void)
{
    if (!status_font) status_font = al_create_builtin_font();

    char buf[32];
    snprintf(buf, sizeof(buf), "%d", score);

    int txt_w = al_get_text_width(status_font, buf);
    int txt_h = al_get_font_line_height(status_font);
    const int PAD = 6, R = 6;

    float x2 = WIDTH - 10;
    float y_turn = 10 + txt_h + 8;     
    float x1 = x2 - txt_w - PAD*2;
    float y1 = y_turn - PAD;

    al_draw_filled_rounded_rectangle(
        x1, y1,
        x2, y_turn + txt_h + PAD,
        R, R, al_map_rgba(0,0,0,160)
    );
    al_draw_text(status_font, al_map_rgb(200,255,255),
                 WIDTH - 10, y_turn, ALLEGRO_ALIGN_RIGHT, buf);
}
