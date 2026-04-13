#ifndef DAMAGE_TEXT_H
#define DAMAGE_TEXT_H

typedef enum {
    TARGET_MONSTER,
    TARGET_PLAYER
} DamageTarget;

typedef struct {
    int r, c;               // 棋盤格座標
    int damage;             // 傷害數值
    int frames_left;        // 顯示剩下幾幀
     int delay; 
    DamageTarget target;    // 傷害對象（怪或玩家）
} DamageText;

#define MAX_DAMAGE_TEXT 64

void show_damage_text(int r, int c, int damage, DamageTarget target);
void draw_damage_texts(void);

#endif
