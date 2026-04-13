#include "cookie_rain.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "../global.h"

#define MAX_DROPS  50
static CookieDrop drops[MAX_DROPS];
static ALLEGRO_BITMAP *umbrella_img = NULL;
static ALLEGRO_BITMAP *drop_img     = NULL;
int cookie_center_r = 0;
int cookie_center_c = 0;

static void hex_to_screen(int r,int c,float*x,float*y){
    float W=get_hex_w(), V=get_hex_v();
    float xc = rowOffsetTable[5]*W + 5*W + (5%2)*(W/2);
    float yc = 5*V;
    float ox = WIDTH/2 - xc, oy = HEIGHT/2 - yc;
    *x = ox + rowOffsetTable[r]*W + c*W + (r%2)*(W/2);
    *y = oy + r*V;
}

void init_cookie_rain(void){
    umbrella_img = al_load_bitmap("assets/image/cookie_umbrella.png");
    drop_img     = al_load_bitmap("assets/image/cookie_drop.png");
    for(int i=0;i<MAX_DROPS;i++) drops[i].ttl=0;
}

void spawn_cookie_rain(int center_r, int center_c){
    cookie_center_r = center_r;
    cookie_center_c = center_c;

    
    float cx, cy;
    hex_to_screen(center_r, center_c, &cx, &cy);
    cy -= get_hex_h() * 0.5f;  


    bool visited[ROWS][12] = {{0}};
    int  dist[ROWS][12]    = {{0}};
    typedef struct { int r, c; } Pos;
    Pos queue[ROWS * 12];
    int head = 0, tail = 0;

    visited[center_r][center_c] = true;
    dist[center_r][center_c]    = 0;
    queue[tail++] = (Pos){center_r, center_c};

    while (head < tail) {
        Pos p = queue[head++];
        int d0 = dist[p.r][p.c];
        if (d0 >= 2) continue;
        for (int d = 0; d < 6; d++) {
            int nr, nc;
            get_neighbor(p.r, p.c, d, &nr, &nc);
            if (nr<0 || nr>=ROWS || nc<0 || nc>=rowLengths[nr]) continue;
            if (visited[nr][nc]) continue;
            visited[nr][nc] = true;
            dist[nr][nc]    = d0 + 1;
            queue[tail++]   = (Pos){nr, nc};
        }
    }

   
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < rowLengths[r]; c++) {
           
            if (!visited[r][c] || (r == center_r && c == center_c)) 
                continue;

            for (int i = 0; i < MAX_DROPS; i++) {
                if (drops[i].ttl <= 0) {
                    float tx, ty;
                    hex_to_screen(r, c, &tx, &ty);
                   
                    ty -= get_hex_h() * 0.4f;

                    drops[i].x   = cx;
                    drops[i].y   = cy;
                    drops[i].tx  = tx;
                    drops[i].ty  = ty;
                    drops[i].ttl = 30; 
                    break;
                }
            }
        }
    }
}

void update_cookie_rain(void){
    for(int i=0;i<MAX_DROPS;i++){
        if(drops[i].ttl>0){
            float t = (float)drops[i].ttl/40.0f;
            drops[i].x = drops[i].tx + (drops[i].x - drops[i].tx)*t;
            drops[i].y = drops[i].ty + (drops[i].y - drops[i].ty)*t;
            drops[i].ttl--;
        }
    }
}

void draw_cookie_rain(void){

    bool raining=false;
    for(int i=0;i<MAX_DROPS;i++) if(drops[i].ttl>0){ raining=true; break; }
    if(raining && umbrella_img){
        // 只畫一次在中心，使用同一個傘座標
        // assume last spawn 傘座標保存在全局或重傳參數
        extern int cookie_center_r, cookie_center_c;
        float cx, cy; hex_to_screen(cookie_center_r, cookie_center_c, &cx, &cy);
        al_draw_scaled_bitmap(umbrella_img, 0,0,
            al_get_bitmap_width(umbrella_img),
            al_get_bitmap_height(umbrella_img),
            cx - get_hex_w(), cy - get_hex_h(), // 一傘覆蓋大範圍
            get_hex_w()*2, get_hex_h()*2, 0);
    }
    //畫所有下落餅乾
    if(drop_img){
        for(int i=0;i<MAX_DROPS;i++){
            if(drops[i].ttl>0){
                float scale = 0.3f + 0.7f*(1.0f - (float)drops[i].ttl/40.0f);
                int w = al_get_bitmap_width(drop_img);
                int h = al_get_bitmap_height(drop_img);
                float dw = get_hex_w()*scale;
                float dh = get_hex_h()*scale;
                al_draw_scaled_bitmap(drop_img,
                    0,0,w,h,
                    drops[i].x-dw/2, drops[i].y-dh/2,
                    dw, dh, 0);
            }
        }
    }
}
