// bubble.h
#ifndef BUBBLE_H
#define BUBBLE_H

#include "../board/hexboard.h"

typedef struct {
    float x, y;        // 當前螢幕座標
    float tx, ty;      // 目標螢幕座標（怪物頭頂）
    int   ttl;         // 剩餘幀數
} MintBubble;

void init_bubbles(void);
void spawn_mint_bubble(int from_r, int from_c, int to_r, int to_c);
void update_bubbles(void);
void draw_bubbles(void);

#endif
