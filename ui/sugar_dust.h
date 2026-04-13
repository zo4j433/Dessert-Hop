#ifndef UI_SUGAR_DUST_H
#define UI_SUGAR_DUST_H

#include "../board/hexboard.h"

// 初始化，游戏启动时调用
void init_sugar_dust(void);

// 每次 auto-move 开始时调用，清除上一回合的糖粉
void clear_sugar_dust(void);

// 在 jl_sugar_dust 里调用，记录每条路径上的格子
void add_sugar_dust_tile(int r, int c);

// 绘制所有糖粉格子（放在 draw_board() 之后）
void draw_sugar_dust(void);

// 怪物移动到某格时调用，检查并对怪物造成糖粉伤害
void apply_sugar_dust_to_monster(int r, int c);

#endif // UI_SUGAR_DUST_H
