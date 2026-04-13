#ifndef STAR_EXPLOSION_H
#define STAR_EXPLOSION_H

// 在怪物死亡時呼叫，(r,c) 是棋盤座標
void spawn_star_explosion(int r, int c);

// 在初始化時呼叫
void init_star_explosion(void);

// 每幀更新時呼叫
void update_star_explosion(void);

// 每幀繪製時呼叫（放在 draw_monsters、draw_player 之後）
void draw_star_explosion(void);

#endif // STAR_EXPLOSION_H
