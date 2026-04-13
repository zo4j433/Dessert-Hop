// monster/CornDemonKing.c

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../global.h"
#include "../board/hexboard.h"    // get_hex_w, get_hex_v, get_neighbor, rowOffsetTable
#include "../system/turn_system.h" // get_current_turn
#include "../ui/damagetext.h"     // show_damage_text
#include "../ui/pebble.h"         // spawn_pebble
#include "../ui/healthbar.h"      // draw_health_bar
#include "CornDemonKing.h"        // Boss struct + prototypes
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "monster.h"

Boss CornDemonKing = { .spawned = false, .alive = false };


bool CornDemonKing_occupies(int r, int c) {
    if (!CornDemonKing.spawned || !CornDemonKing.alive) return false;
    for (int i = 0; i < CORN_CELLS; i++) {
        if (CornDemonKing.occ_r[i] == r && CornDemonKing.occ_c[i] == c)
            return true;
    }
    return false;
}


static void get_screen_pos_local(int r, int c, float *out_x, float *out_y) {
    float HEX_W = get_hex_w(), HEX_V = get_hex_v();
    float x_center = rowOffsetTable[5]*HEX_W + 5*HEX_W + (5%2)*(HEX_W/2);
    float y_center = 5*HEX_V;
    float ox = WIDTH/2 - x_center;
    float oy = HEIGHT/2 - y_center;
    float row_off = rowOffsetTable[r]*HEX_W;
    *out_x = ox + row_off + c*HEX_W + (r%2)*(HEX_W/2);
    *out_y = oy + r*HEX_V;
}


static void compute_occupied_cells_for(int cr, int cc, int out_r[CORN_CELLS], int out_c[CORN_CELLS]) {
    out_r[0] = cr; out_c[0] = cc;
    for (int d = 0; d < 6; d++) {
        get_neighbor(cr, cc, d, &out_r[d+1], &out_c[d+1]);
    }
}


static void compute_occupied_cells(void) {
    compute_occupied_cells_for(CornDemonKing.r, CornDemonKing.c,
                              CornDemonKing.occ_r, CornDemonKing.occ_c);
}


static void init_CDK(int cr, int cc) {
    Boss *b = &CornDemonKing;
    b->r = cr; b->c = cc;
    b->max_hp = 2400; b->hp = b->max_hp;
    b->atk = 60;
    b->move_range = 2;
    b->range = 3;
    b->speed = 4.0f;
    b->attack_delay = 0;
    b->moving = false;
    b->alive = true;
    compute_occupied_cells();
    get_screen_pos_local(b->r, b->c, &b->sx, &b->sy);
    b->tx = b->sx; b->ty = b->sy;
}


static void skill_fire_popcorn(int pr, int pc) {
    Boss *b = &CornDemonKing;
    int dmg = b->atk * 120 / 100;
    float tx, ty;
    get_screen_pos_local(pr, pc, &tx, &ty);
    spawn_pebble(b->sx, b->sy, tx, ty);
    show_damage_text(pr, pc, dmg, TARGET_PLAYER);
}
static void skill_jump_smash(int pr, int pc) {
    int dmg = CornDemonKing.atk * 160 / 100;
    show_damage_text(pr, pc, dmg, TARGET_PLAYER);
}


void CornDemonKing_update_and_act(int player_r, int player_c, int *player_hp) {
    if (!CornDemonKing.spawned) {
        int temp_r[CORN_CELLS], temp_c[CORN_CELLS];
        bool found = false;
        int cr=0, cc=0;
        for (int r=0; r<ROWS && !found; r++) {
            for (int c=0; c<rowLengths[r] && !found; c++) {
                compute_occupied_cells_for(r,c,temp_r,temp_c);
                bool ok=true;
                for(int i=0;i<CORN_CELLS;i++){
                    if(temp_r[i]<0||temp_r[i]>=ROWS||temp_c[i]<0||temp_c[i]>=rowLengths[temp_r[i]]){
                        ok=false;break;
                    }
                }
                if(!ok) continue;
                bool overlap=false;
                for(int i=0;i<CORN_CELLS;i++){
                    if(is_monster_at(temp_r[i],temp_c[i])){overlap=true;break;}
                }
                if(!overlap){ cr=r; cc=c; found=true; }
            }
        }
        if(found){ init_CDK(cr,cc); CornDemonKing.spawned=true; printf("★ Corn Demon King spawned at (%d,%d)\n", cr,cc); }
        return;
    }
    if(!CornDemonKing.alive) return;
    Boss *b=&CornDemonKing;
    compute_occupied_cells();
    int dist = abs(b->r-player_r)+abs(b->c-player_c);
    if(dist<=b->range){ skill_fire_popcorn(player_r,player_c); }
    else if(dist==1){ skill_jump_smash(player_r,player_c); }


else if (dist > b->range && dist > 1) {
    // 每回合最多走 b->move_range 步
    for (int step = 0; step < b->move_range; step++) {
        int best_r = b->r, best_c = b->c;
        int best_d = dist;

       
        for (int d = 0; d < 6; d++) {
            int nr, nc;
            get_neighbor(b->r, b->c, d, &nr, &nc);
            if (nr<0||nr>=ROWS||nc<0||nc>=rowLengths[nr]) continue;
            if (player_r==nr && player_c==nc) continue;
            int d2 = abs(nr-player_r) + abs(nc-player_c);
            if (d2 < best_d) {
                best_d = d2;
                best_r = nr;
                best_c = nc;
            }
        }
        if (best_r == b->r && best_c == b->c) break;

        //殺掉目標格上的小怪
        if (is_monster_at(best_r, best_c)) {
            damage_monster_at(best_r, best_c, 9999);
        }

        // 更新 Boss 位置
        b->r = best_r;
        b->c = best_c;
        compute_occupied_cells();
        for (int i = 0; i < CORN_CELLS; i++) {
        int mr = b->occ_r[i];
        int mc = b->occ_c[i];
    
}

        get_screen_pos_local(best_r, best_c, &b->tx, &b->ty);
        b->moving = true;
        b->attack_delay = 20;
    }
}

}

// Draw
void CornDemonKing_draw(void){
    if(!CornDemonKing.spawned||!CornDemonKing.alive) return;
    static ALLEGRO_BITMAP *bmp=NULL;
    if(!bmp) bmp=al_load_bitmap("assets/image/monsters/giant_corn.png");
    Boss *b=&CornDemonKing;
    float cx=b->sx, cy=b->sy;
    int w=al_get_bitmap_width(bmp), h=al_get_bitmap_height(bmp);
    float cell_w=get_hex_w()*0.9f;
    for(int i=0;i<CORN_CELLS;i++){
        int gr=b->occ_r[i]-b->r, gc=b->occ_c[i]-b->c;
        float dx=(rowOffsetTable[b->r+gr]-rowOffsetTable[b->r])*get_hex_w()
                 +gc*get_hex_w()+(((b->r+gr)%2)-(b->r%2))*(get_hex_w()/2);
        float dy=gr*get_hex_v();
        float x=cx+dx, y=cy+dy;
        float dh=cell_w*(h/(float)w);
        al_draw_scaled_bitmap(bmp,0,0,w,h,x-cell_w/2,y-dh/2,cell_w,dh,0);
    }
    draw_health_bar(b->sx-40,b->sy-50,80,8,b->hp,b->max_hp);
}

// Animation update
#include <allegro5/allegro_primitives.h>
void CornDemonKing_update_animation(void){
    Boss *b=&CornDemonKing;
    if(!b->moving) return;
    float dx=b->tx-b->sx, dy=b->ty-b->sy;
    float dist=sqrtf(dx*dx+dy*dy);
    if(dist<=b->speed){ b->sx=b->tx; b->sy=b->ty; b->moving=false; }
    else{ b->sx+=dx/dist*b->speed; b->sy+=dy/dist*b->speed; }
}