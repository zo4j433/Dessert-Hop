#ifndef TURN_SYSTEM_H
#define TURN_SYSTEM_H

typedef enum {
    TURN_PLAYER,
    TURN_MONSTER
} TurnState;

extern int current_turn;
extern TurnState turn_state;
extern int  combo_hits;           // 本回合踩到怪的次數
extern float combo_bonus;         // 當前攻擊倍率 (>=1.0)
extern int  raw_pts_this_turn;    // 本回合尚未乘 combo 的原始分數
extern int  score;                // 總積分

void next_turn();         // 切換回合
void monster_turn();      // 怪物行動
int get_current_turn(void);

void draw_combo_ui(void);
void draw_score_ui(void);
#endif
