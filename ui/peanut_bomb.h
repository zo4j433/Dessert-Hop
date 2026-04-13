#ifndef UI_PEANUT_BOMB_H
#define UI_PEANUT_BOMB_H

#include "../board/hexboard.h"

typedef struct {
    bool   active;
    float  x, y;      // 當前螢幕座標
    float  vx, vy;    // 速度
    int    ttl;       // 剩餘帧數
    int    tr, tc;    // 目标格子棋盤座標（用來產生爆炸特效或音效）
} PeanutBomb;

#define MAX_PEANUT_BOMBS 8

// 初始化，遊戲開始時呼叫一次
void init_peanut_bombs(void);

// 每幀更新，遊戲主迴圈 update 階段呼叫
void update_peanut_bombs(void);

// 每幀繪製，draw_board() 之後呼叫
void draw_peanut_bombs(void);

// 丟一顆花生炸彈：從 (sr,sc) 掉到 (tr,tc)
void spawn_peanut_bomb(int tr, int tc);

#endif // UI_PEANUT_BOMB_H
