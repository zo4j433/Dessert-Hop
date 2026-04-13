// player.h
#ifndef PLAYER_H
#define PLAYER_H
#include <stdbool.h>  

#include "../ui/types.h"
typedef struct {
    int r, c;
    int hp;
    int max_hp;
    int level;        
    int atk; 
    int xp;           
    int xp_to_next;  
    float lifesteal; 
    float damage_multiplier; 
    float damage_reduction;
    float damage_multiplier2; 
    float counter_multiplier;   
    int   last_attacker_r, last_attacker_c;  
} Player;

//畫線線
void draw_walked_paths(void);


extern Player player;



void init_player();
void move_player_to_mouse(int mouse_x, int mouse_y);
void draw_player();


// 走過的路徑
#define MAX_PATH_LEN 64

extern Pos move_path[MAX_PATH_LEN];
extern int path_len;

// 自動移動狀態
extern bool auto_moving;
extern int auto_index;
extern int auto_tick_delay;
void player_update(void);

//不能回走
bool is_path_blocked(int r1,int c1,int r2,int c2);
bool is_player_at(int row, int col);
void record_walked_path(int r1,int c1,int r2,int c2);
void reset_walked_paths(void);

int undo_last_step(void);

void damage_player(int dmg);

void gain_xp(int amount);

void init_player_graphics(void);

void destroy_player_graphics(void);
#endif
