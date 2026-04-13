#include "global.h"
#include "shapes/Shape.h"
#include <stdbool.h>
// variables for global usage
#include "global.h"

bool game_paused = false;  
const double FPS = 60.0;
const int WIDTH = 1024;
const int HEIGHT = 720;
int window = 0;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_EVENT event;
ALLEGRO_TIMER *fps = NULL;
bool key_state[ALLEGRO_KEY_MAX] = {false};
bool mouse_state[ALLEGRO_MOUSE_MAX_EXTRA_AXES] = {false};
Point mouse;
bool debug_mode = true;
bool game_won = false;
int final_score = 0;
bool exit_requested = false;
//pp
//pp
//pp
//pp
//pp
//pp1pp