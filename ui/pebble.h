#ifndef PEBBLE_H
#define PEBBLE_H

#include <allegro5/allegro_primitives.h>

#define MAX_PEBBLES 64

typedef struct {
    float x, y;        /* 目前螢幕座標 */
    float vx, vy;      /* 每禎位移 */
    int   frames_left; /* 存活禎數 */
} Pebble;

/* 產生一顆從 (sx,sy) → (tx,ty) 的黑石 */
void spawn_pebble(float sx,float sy,float tx,float ty);

/* 每禎呼叫：更新座標 / 壽命 */
void update_pebbles(void);

/* 每禎呼叫：繪製所有黑石 */
void draw_pebbles(void);

#endif
