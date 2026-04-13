#include "gloopie.h"
#include "../board/hexboard.h"
#include "../monster/monster.h"
#include "../player/player.h"
#include "../board/movable.h"
#include "../player/player.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int row, col;
} Gloopie;

static Gloopie gloopies[7];  // 儲存七個 gloopie

void init_gloopie() {
    // 初始排列
    int base_r = 5, base_c = 5;
    int idx = 0;
    int offsets[7][2] = {
        {-1, -1}, {-1, 0},
        { 0, -1}, { 0, 0}, { 0, 1},
        { 1, -1}, { 1, 0}
    };
    for (int i = 0; i < 7; i++) {
        int r = base_r + offsets[i][0];
        int c = base_c + offsets[i][1];
        gloopies[idx++] = (Gloopie){r, c};
    }
}

void scatter_gloopie() {
    srand(time(NULL));
    int count = 0;
    while (count < 7) {
        int r = rand() % ROWS;
        int c = rand() % rowLengths[r];
        if (!is_monster_at(r, c) && !is_player_at(r, c) && is_surrounding_safe(r, c)) {
            // 檢查是否重複
            bool duplicated = false;
            for (int i = 0; i < count; i++) {
                if (gloopies[i].row == r && gloopies[i].col == c) {
                    duplicated = true;
                    break;
                }
            }
            if (!duplicated) {
                gloopies[count++] = (Gloopie){r, c};
            }
        }
    }
}

void update_gloopie() {
   
}

void draw_gloopie() {
    for (int i = 0; i < 7; i++) {
        int r = gloopies[i].row;
        int c = gloopies[i].col;
        float cx, cy;
        get_hex_center(r, c, &cx, &cy);
        al_draw_filled_circle(cx, cy, 18, al_map_rgb(0, 255, 0));
    }
}


void destroy_gloopie() {
    
}
