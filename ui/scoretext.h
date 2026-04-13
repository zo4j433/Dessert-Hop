#ifndef SCORE_TEXT_H
#define SCORE_TEXT_H

#define MAX_SCORE_TEXT 16

// 每次 score 增加呼叫
void show_score_text(int pts);

// 在 game_draw() 最後呼叫
void draw_score_texts(void);

#endif
