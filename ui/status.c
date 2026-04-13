#include "status.h"
#include "../global.h"
#include "../player/player.h"
#include "../system/turn_system.h"  
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>

 
static ALLEGRO_BITMAP *avatar = NULL;

void init_player_status_ui(void) {
    if (!avatar) {
        avatar = al_load_bitmap("assets/image/player_head.png");
    }
    if (!status_font) {
        
        status_font = al_load_ttf_font("assets/font/pirulen.ttf", 28, 0);
        
    }
}

void destroy_player_status_ui(void) {
    if (avatar) {
        al_destroy_bitmap(avatar);
        avatar = NULL;
    }
    if (status_font) {
        al_destroy_font(status_font);
        status_font = NULL;
    }
}

void draw_player_status(void)
{
    // 确保 avatar 已经加载
     if (!avatar || !status_font) init_player_status_ui();

    const float M = 20;                        
    const float ICON = 150;                    
    const float BAR_W = 150, BAR_H = 12;
    const float GAP = 8;

    float x0 = M;
    float y0 = HEIGHT - M - ICON-20;

    // 1) 头像
    al_draw_scaled_bitmap(avatar,
                          0,0,
                          al_get_bitmap_width(avatar),
                          al_get_bitmap_height(avatar),
                          x0, y0,
                          ICON, ICON,
                          0);
    char buf[16];
    snprintf(buf, sizeof(buf), "Lv %d", player.level);

       // 文字尺寸
    int txt_w = al_get_text_width(status_font, buf);
    int txt_h = al_get_font_line_height(status_font);

    // 文本起点
    float tx = x0  + GAP;
    float ty = y0 ;

    // 背景框 padding 和圆角半径
    const float PAD = 6;
    const float R   = 8;

    // 画一个半透明深色圆角框
    al_draw_filled_rounded_rectangle(
        tx - PAD, ty - PAD,
        tx + txt_w + PAD, ty + txt_h + PAD,
        R, R,
        al_map_rgba(0, 0, 0, 160)
    );

    // 然后再画文字
    al_draw_text(status_font,
                 al_map_rgb(255,200,200),
                 tx, ty,
                 ALLEGRO_ALIGN_LEFT,
                 buf);

    // 3) 经验条
    float bx = x0;
    float by = y0 + ICON + GAP-30;
    float radius = BAR_H/2;
    al_draw_filled_rounded_rectangle(bx, by,
                                     bx + BAR_W, by + BAR_H,
                                     radius, radius,
                                     al_map_rgb(250,230,250));  // 浅紫背景
    // 4) 画圆角进度条
    float ratio = (float)player.xp / player.xp_to_next;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;
    al_draw_filled_rounded_rectangle(bx, by,
                                     bx + BAR_W * ratio,
                                     by + BAR_H,
                                     radius, radius,
                                     al_map_rgb(255,100,180));  // 亮粉前景
}

void draw_turn(void) {
    if (!status_font) status_font = al_create_builtin_font();

    char buf[32];
    snprintf(buf, sizeof(buf), "Turn %d", current_turn);

    // 測量文字尺寸
    int txt_w = al_get_text_width(status_font, buf);
    int txt_h = al_get_font_line_height(status_font);

    // 背景框 padding 與圓角半徑
    const float PAD = 6;
    const float R   = 6;

    // 計算背景框座標（右上角對齊）
    float x2 = WIDTH - 10;            // 文字右邊緣
    float y2 = 10 + txt_h;            // 文字下邊緣
    float x1 = x2 - txt_w - PAD*2;    // 左邊緣
    float y1 = 10 - PAD;              // 上邊緣

    // 1) 畫半透明圓角背景
    al_draw_filled_rounded_rectangle(
        x1, y1,
        x2, y2 + PAD,
        R, R,
        al_map_rgba(0, 0, 0, 160)
    );

    // 2) 再畫文字（保持右上對齊）
    al_draw_text(
        status_font,
        al_map_rgb(255, 255, 255),
        WIDTH - 10, 10,
        ALLEGRO_ALIGN_RIGHT,
        buf
    );
}

