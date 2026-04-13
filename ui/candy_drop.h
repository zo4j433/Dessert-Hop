#ifndef UI_CANDY_DROP_H
#define UI_CANDY_DROP_H

#include "../board/hexboard.h"

typedef struct {
    bool   active;
    float  x, y;    
    float  vy;      
    int    tr, tc;  
} CandyDrop;

#define MAX_CANDY_DROPS 16


void init_candy_drop(void);


void update_candy_drop(void);


void draw_candy_drop(void);


void spawn_candy_drop(int r, int c);

#endif 
