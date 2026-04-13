#include "pebble.h"
#include <math.h>
#include "../global.h"

static Pebble plist[MAX_PEBBLES];

void spawn_pebble(float sx,float sy,float tx,float ty)
{
    for(int i=0;i<MAX_PEBBLES;i++){
        if(plist[i].frames_left<=0){
            float dx=tx-sx, dy=ty-sy;
            float len=sqrtf(dx*dx+dy*dy); if(len==0) len=1;
            float speed = 6;                        /* px/ frame */
            plist[i]=(Pebble){
                .x=sx, .y=sy,
                .vx=dx/len*speed,
                .vy=dy/len*speed,
                .frames_left=(int)(len/speed)+3
            };
            return;
        }
    }
}

void update_pebbles(void)
{
    for(int i=0;i<MAX_PEBBLES;i++){
        if(plist[i].frames_left>0){
            plist[i].x += plist[i].vx;
            plist[i].y += plist[i].vy;
            plist[i].frames_left--;
        }
    }
}

void draw_pebbles(void)
{
    for(int i=0;i<MAX_PEBBLES;i++){
        if(plist[i].frames_left>0){
            al_draw_filled_circle(plist[i].x, plist[i].y, 4,
                                   al_map_rgb(30,30,30));
        }
    }
}
