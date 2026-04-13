#ifndef COOKIE_RAIN_H
#define COOKIE_RAIN_H

#include "../board/hexboard.h"

typedef struct {
    float x, y;      // 當前座標
    float tx, ty;    // 目標座標（落點格子中心）
    int   ttl;       // 剩餘幀數
} CookieDrop;

void init_cookie_rain(void);
void spawn_cookie_rain(int center_r, int center_c);
void update_cookie_rain(void);
void draw_cookie_rain(void);

extern int cookie_center_r;
extern int cookie_center_c;

#endif
