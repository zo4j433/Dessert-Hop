#ifndef HEXBOARD_H
#define HEXBOARD_H

#include <allegro5/allegro_font.h>
#include "../ui/types.h"

// 棋盤規格
#define ROWS 11
extern float HEX_SIZE;                          // 定義在 hexboard.c
extern const int rowLengths[ROWS];
extern const int rowOffsetTable[ROWS];

float get_hex_w(void);
float get_hex_h(void);
float get_hex_v(void);

extern ALLEGRO_FONT *hex_font;                // init_board 會建立

void init_board(void);                        // GameWindow 會呼叫
void draw_board(void);
void get_hex_center(int row, int col, float *out_x, float *out_y); // 場景繪圖呼叫

// 玩家可移動格 
#define MAX_MOVES 12
extern Pos movable_tiles[MAX_MOVES];
extern int movable_cnt;

bool is_surrounding_safe(int r,int c);
void get_neighbor(int r, int c, int dir, int *nr, int *nc);
#endif