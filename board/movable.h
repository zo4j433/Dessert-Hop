#ifndef MOVABLE_H
#define MOVABLE_H

#include "hexboard.h"          /* ROWS, rowLengths, rowOffsetTable */
#include "../player/player.h"   /* player 位置 */
#include "../monster/monster.h" /* is_monster_at */
#include "../ui/types.h"
#define MAX_JUMPS 6
void update_movable_tiles(void);
extern int  movable_cnt, jump_cnt;
extern bool first_step, jump_enabled;
extern Pos  movable_tiles[MAX_MOVES];
extern Pos  jump_tiles[MAX_JUMPS];

/* 回傳該格是不是在 movable_tiles 裡 */
int is_movable(int r, int c);

#endif
