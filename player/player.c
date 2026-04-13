#include "player.h"
#include "../global.h"
#include "../board/hexboard.h" /* get_hex_w / get_hex_v / rowOffsetTable */
#include "../board/movable.h"  /* update_movable_tiles, is_movable */
#include <allegro5/allegro_primitives.h>
#include "../board/movable.h"
#include "../monster/monster.h"
#include "../system/turn_system.h"
#include "../ui/damagetext.h"
#include "../ui/healthbar.h"
#include "../skill/skill.h"
#include "../ui/star_explosion.h"
#include "../ui/healtext.h"
#include "../ui/sugar_dust.h"
#include <math.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>

extern ALLEGRO_SAMPLE *jump_sample;
extern ALLEGRO_SAMPLE *attack_sample;

static ALLEGRO_BITMAP *player_bitmap = NULL;

Player player;
void init_player_graphics(void)
{
    if (!player_bitmap)
    {
        player_bitmap = al_load_bitmap("assets/image/player.png");
        if (!player_bitmap)
        {
            fprintf(stderr, "Failed to load player.png\n");
            exit(1);
        }
    }

    if (!attack_sample)
    {
        attack_sample = al_load_sample("assets/sound/attack.wav");
        if (!attack_sample)
        {
            fprintf(stderr, "Failed to load attack sound!\n");
            // 不退出遊戲，只是沒有音效
        }
    }
}
void destroy_player_graphics(void)
{
    if (player_bitmap)
    {
        al_destroy_bitmap(player_bitmap);
        player_bitmap = NULL;
    }

    if (attack_sample)
    {
        al_destroy_sample(attack_sample);
        attack_sample = NULL;
    }
}

void init_player(void)
{
    first_step = true;
    player.r = 5;
    player.c = 5;
    player.max_hp = player.hp = 500;
    player.level = 1;
    player.xp = 0;
    player.atk = 100;
    player.xp_to_next = 100;
    player.damage_multiplier = 1.0f;
    player.damage_multiplier2 = 1.0f;
    player.lifesteal = 0.0f;
    player.counter_multiplier = 0.0f;
    player.last_attacker_r = -1;
    player.last_attacker_c = -1;
    player.damage_reduction = 0.0f;
}

void gain_xp(int amount)
{
    player.xp += amount;
    while (player.xp >= player.xp_to_next)
    {
        player.xp -= player.xp_to_next;
        player.level++;

        player.xp_to_next += 50;
        player.max_hp += 10;
        player.atk += 5;
        player.hp += 10;
        printf("Player leveled up! Now level %d (next: %d xp)\n",
               player.level, player.xp_to_next);
        toggle_skill_scene();
    }
}

static void player_get_screen_pos(float *sx, float *sy)
{
    float W = get_hex_w(), V = get_hex_v();
    /* 與 draw_board 計算中心格偏移量一致 */
    float x_center = rowOffsetTable[5] * W + 5 * W + (5 % 2) * (W / 2);
    float y_center = 5 * V;
    float ox = WIDTH / 2 - x_center;
    float oy = HEIGHT / 2 - y_center;

    *sx = ox + rowOffsetTable[player.r] * W + player.c * W + (player.r % 2) * (W / 2);
    *sy = oy + player.r * V;
}

void damage_player(int dmg)
{
    int reduced = (int)(dmg * (1.0f - player.damage_reduction));
    player.hp -= reduced;
    if (player.hp < 0)
        player.hp = 0;

    show_damage_text(player.r, player.c, reduced, TARGET_MONSTER);
    printf("Player takes %d dmg, HP=%d\n", reduced, player.hp);
    if (player.counter_multiplier > 0.0f && player.last_attacker_r >= 0)
    {
        int cr = player.last_attacker_r;
        int cc = player.last_attacker_c;
        int counter_dmg = (int)(dmg * player.counter_multiplier + 0.5f);
        printf("[Sweet Counter] counterattack at (%d,%d) for %d dmg\n",
               cr, cc, counter_dmg);
        if (is_monster_at(cr, cc))
        {
            damage_monster_at(cr, cc, counter_dmg);
        }
        player.last_attacker_r = -1;
    }
}

void move_player_to_mouse(int mx, int my)
{

    float HEX_W = get_hex_w(), HEX_V = get_hex_v();

    float x_center = rowOffsetTable[5] * HEX_W + 5 * HEX_W + (5 % 2) * (HEX_W / 2);
    float y_center = 5 * HEX_V;
    float ox = WIDTH / 2 - x_center;
    float oy = HEIGHT / 2 - y_center;

    float map_x = mx - ox;
    float map_y = my - oy;

    int est_r = (int)(map_y / HEX_V + 0.5f);
    if (est_r < 0 || est_r >= ROWS)
        return;

    float row_off = rowOffsetTable[est_r] * HEX_W + (est_r % 2) * (HEX_W / 2);
    int est_c = (int)((map_x - row_off) / HEX_W + 0.5f);
    if (est_c < 0 || est_c >= rowLengths[est_r])
        return;

    if (path_len == 0)
    {
        move_path[path_len++] = (Pos){player.r, player.c};
    }

    if (is_movable(est_r, est_c))
    {
        int prev_r = player.r;
        int prev_c = player.c;
        player.r = est_r;
        player.c = est_c;
        record_walked_path(prev_r, prev_c, player.r, player.c);
        movable_cnt = 0;
        jump_cnt = 0;
        jump_enabled = false;
        first_step = false;
        if (path_len < MAX_PATH_LEN)
            move_path[path_len++] = (Pos){est_r, est_c};

        // 音效
        if (jump_sample)
        {
            al_play_sample(jump_sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }
        return;
    }

    for (int i = 0; i < jump_cnt; i++)
    {
        if (jump_tiles[i].r == est_r && jump_tiles[i].c == est_c)
        {
            int prev_r = player.r;
            int prev_c = player.c;
            player.r = est_r;
            player.c = est_c;
            record_walked_path(prev_r, prev_c, player.r, player.c);
            first_step = false;

            if (path_len < MAX_PATH_LEN)
                move_path[path_len++] = (Pos){est_r, est_c};

            combo_hits++;
            if (!auto_moving)
                update_movable_tiles();

            // 音效
            if (jump_sample)
            {
                al_play_sample(jump_sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }

            return;
        }
    }
}

void draw_player(void)
{
    float HEX_W = get_hex_w(), HEX_V = get_hex_v();

    // 棋盤座標->螢幕座標
    float row_off = rowOffsetTable[player.r] * HEX_W;
    float cx_hex = row_off + player.c * HEX_W + (player.r % 2) * (HEX_W / 2);
    float cy_hex = player.r * HEX_V;

    float x_center = rowOffsetTable[5] * HEX_W + 5 * HEX_W + (5 % 2) * (HEX_W / 2);
    float y_center = 5 * HEX_V;
    float ox = WIDTH / 2 - x_center;
    float oy = HEIGHT / 2 - y_center;

    float x = ox + cx_hex;
    float y = oy + cy_hex;

    /* 玩家：藍色圓 */
    // al_draw_filled_circle(x, y, HEX_SIZE * 0.35f, al_map_rgb(80, 160, 255));
    // al_draw_circle(x, y, HEX_SIZE * 0.35f,        al_map_rgb(0, 80, 160), 2);
    int w = al_get_bitmap_width(player_bitmap);
    int h = al_get_bitmap_height(player_bitmap);
    float display_w = HEX_W * 1.1f;
    float display_h = display_w * ((float)h / w);

    // 繪製玩家貼圖，中心對齊
    al_draw_scaled_bitmap(
        player_bitmap,
        0, 0, w, h,
        x - display_w / 2,
        y - display_h / 2 - 10,
        display_w, display_h,
        0);
    draw_health_bar(x - 25, y - HEX_SIZE * 1.1f, 50, 6, player.hp, player.max_hp);
}

/*
   找出從 (sr,sc) 到 (er,ec) 的方向編號 d (0‥5)。
   如果 er,ec 不在同一直線，回傳 -1。               */
static int find_direction(int sr, int sc, int er, int ec)
{
    for (int d = 0; d < 6; d++)
    {
        int r = sr, c = sc;
        while (1)
        {
            int nr, nc;
            get_neighbor(r, c, d, &nr, &nc);

            if (nr == er && nc == ec)
                return d;
            if (nr < 0 || nr >= ROWS)
                break;
            if (nc < 0 || nc >= rowLengths[nr])
                break;

            r = nr;
            c = nc;
        }
    }
    return -1;
}

Pos move_path[MAX_PATH_LEN];
int path_len = 0;

bool auto_moving = false;
int auto_index = 0;
int auto_tick_delay = 0; // 每幾幀走一步（延遲用）

void player_update(void)
{
    if (is_skill_scene_visible())
        return;
    if (!auto_moving)
        return;

    auto_tick_delay++;
    if (auto_tick_delay < 40)
        return; // 速度控制
    auto_tick_delay = 0;

    if (auto_index >= path_len)
    {
        auto_moving = false;
        auto_index = 0;
        next_turn();
        return;
    }

    int nr = move_path[auto_index].r;
    int nc = move_path[auto_index].c;

    int d = find_direction(player.r, player.c, nr, nc); /* 0‥5 or -1 */

    if (d != -1 && auto_index != 0)
    {
        int r = player.r, c = player.c;
        while (1)
        {
            int tr, tc;
            get_neighbor(r, c, d, &tr, &tc);
            if (tr == nr && tc == nc)
                break; // 終點，不扣血

            if (is_monster_at(tr, tc))
            {
                combo_hits++;

                int extra = combo_hits - 4;
                combo_bonus = (extra > 0) ? 1.0f + extra * 0.04f : 1.0f;
                int dmg = (int)(player.atk * combo_bonus * player.damage_multiplier + 0.5f);

                // 回寫技能
                if (player.lifesteal > 0.0f)
                {
                    int heal = (int)(dmg * player.lifesteal + 0.5f);
                    show_heal_text(nr, nc, heal);

                    player.hp += heal;
                    if (player.hp > player.max_hp)
                        player.hp = player.max_hp;

                    printf("[Cream Lifesteal] healed %d HP (%.2f%% of %d dmg), HP=%d/%d\n",
                           heal, player.lifesteal * 100.0f, dmg, player.hp, player.max_hp);
                }

                if (attack_sample)
                {
                    al_play_sample(attack_sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                damage_monster_at(tr, tc, dmg);

                if (!is_monster_at(tr, tc))
                {
                    // 代表這次普通攻擊擊殺了怪，觸發 Matcha Core
                    for (int i = 0; i < selected_count; i++)
                    {
                        Skill *sk = selected_skills[i].skill;
                        if (sk->id == SKL_MATCHA_CORE && sk->on_jump_land)
                        {
                            spawn_star_explosion(tr, tc);
                            sk->on_jump_land(sk, tr, tc);
                        }
                    }
                }
            }
            else
            {
                printf("MISS (%d,%d)\n", tr, tc);
            }
            apply_sugar_dust_to_monster(tr, tc);
            r = tr;
            c = tc;
        }
    }

    // 真正移動玩家
    player.r = nr;
    player.c = nc;
    printf("player at (%d,%d)\n", nr, nc);
    if (auto_index != 0)
    {

        for (int i = 0; i < selected_count; i++)
        {
            Skill *s = selected_skills[i].skill;
            // 如果這招沒有 on_jump_land，就跳過
            if (!s->on_jump_land)
                continue;

            // Cream Cookie 只在最後落點觸發
            if (s->id == SKL_CREAM_COOKIE)
            {
                if (auto_index == path_len - 1)
                {
                    s->on_jump_land(s, nr, nc);
                }
            }
            // 其他技能每格都觸發
            else if (s->id != SKL_MATCHA_CORE)
            {
                s->on_jump_land(s, nr, nc);
            }
        }
    }
    else
    {
        clear_sugar_dust();
    }
    auto_index++;
}

// 走過邊的紀錄
#define MAX_PATH_RECORD 120 // 單回合能走(跳)的最大步數

typedef struct
{
    int r1, c1;
    int r2, c2; // A↔B 視為同一條
} PathRecord;

static PathRecord walked_paths[MAX_PATH_RECORD];
static int walked_cnt = 0;

bool is_player_at(int row, int col)
{
    return player.r == row && player.c == col;
}

// 判斷這條邊是否已經走過
bool is_path_blocked(int r1, int c1, int r2, int c2)
{
    for (int i = 0; i < walked_cnt; i++)
    {
        PathRecord p = walked_paths[i];
        if ((p.r1 == r1 && p.c1 == c1 && p.r2 == r2 && p.c2 == c2) ||
            (p.r1 == r2 && p.c1 == c2 && p.r2 == r1 && p.c2 == c1))
            return true;
    }
    return false;
}

// 在完成一次移動後呼叫 — 記錄這條邊
void record_walked_path(int r1, int c1, int r2, int c2)
{
    if (walked_cnt >= MAX_PATH_RECORD)
        return;
    walked_paths[walked_cnt++] = (PathRecord){r1, c1, r2, c2};
}

void reset_walked_paths(void) { walked_cnt = 0; }

// 把本回合已走過的邊畫出來（放在 player.c 任何函式之外）

void draw_walked_paths(void)
{
    float W = get_hex_w();
    float V = get_hex_v();

    int cRow = 5, cCol = 5;
    float x_center = rowOffsetTable[cRow] * W + cCol * W + (cRow % 2) * (W / 2);
    float y_center = cRow * V;
    float ox = WIDTH / 2 - x_center;
    float oy = HEIGHT / 2 - y_center;

    for (int i = 0; i < walked_cnt; i++)
    {
        int r1 = walked_paths[i].r1, c1 = walked_paths[i].c1;
        int r2 = walked_paths[i].r2, c2 = walked_paths[i].c2;

        float x1 = ox + rowOffsetTable[r1] * W + c1 * W + (r1 % 2) * (W / 2);
        float y1 = oy + r1 * V;
        float x2 = ox + rowOffsetTable[r2] * W + c2 * W + (r2 % 2) * (W / 2);
        float y2 = oy + r2 * V;

        al_draw_line(x1, y1, x2, y2,
                     al_map_rgb(254, 255, 213),
                     5);
    }
}
// Backspace 觸發：將玩家退回上一個手動點的格子。
int undo_last_step(void)
{
    // move_path[0] 是起點，只有 1 筆代表沒走過任何格
    if (auto_moving || path_len <= 1)
        return 0;

    /// 目前所在點 = move_path[path_len-1]
    // 上一步      = move_path[path_len-2]
    int cur_r = move_path[path_len - 1].r;
    int cur_c = move_path[path_len - 1].c;
    int prv_r = move_path[path_len - 2].r;
    int prv_c = move_path[path_len - 2].c;

    /// 把玩家位置拉回上一步
    player.r = prv_r;
    player.c = prv_c;

    path_len--;
    combo_hits--;
    if (walked_cnt)
        walked_cnt--;
    if (path_len == 1)
    {
        first_step = true;
        jump_enabled = true;
    }
    update_movable_tiles();
    return 1;
}
