#include <allegro5/allegro_primitives.h>
#include "healthbar.h"

void draw_health_bar(float x, float y, float width, float height, int hp, int max_hp) {
    if (max_hp <= 0) return; // 避免除以 0

    float ratio = (float)hp / max_hp;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    // 底層灰條（背景）
    al_draw_filled_rectangle(x, y, x + width, y + height, al_map_rgb(120, 120, 120));
    // 血量紅條（前景）
    al_draw_filled_rectangle(x, y, x + width * ratio, y + height, al_map_rgb(255, 60, 60));
    // 邊框
    al_draw_rectangle(x, y, x + width, y + height, al_map_rgb(0, 0, 0), 1.0);
}
