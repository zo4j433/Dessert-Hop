#include <allegro5/allegro_font.h>
#ifndef UI_STATUS_H
#define UI_STATUS_H

void draw_player_status(void);
void draw_turn(void);
extern ALLEGRO_FONT *status_font ;
void init_player_status_ui();
void destroy_player_status_ui();

#endif
