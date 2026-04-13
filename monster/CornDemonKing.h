#ifndef CORN_DEMON_KING_H
#define CORN_DEMON_KING_H

#include <stdbool.h>

#define CORN_CELLS 7

typedef struct {
    int    r, c;                    // 中心格
    float  sx, sy, tx, ty;
    int    hp, max_hp;
    int    atk;
    int    move_range;
    int    range;
    float  speed;
    int    attack_delay;
    bool   moving;
    bool   spawned;
    bool   alive;

    //佔據的總格座標
    int    occ_r[CORN_CELLS];
    int    occ_c[CORN_CELLS];
} Boss;

// 每回合一開始呼叫
void CornDemonKing_update_and_act(int player_r, int player_c, int *player_hp);

// draw 階段呼叫
void CornDemonKing_draw(void);

// 檢查某格是否被 Boss 佔據
bool CornDemonKing_occupies(int r, int c);

void CornDemonKing_update_animation(); 

extern Boss CornDemonKing;

#endif 
