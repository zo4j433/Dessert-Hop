#include "GameWindow.h"
#include "global.h"
#include "shapes/Shape.h"
#include "GAME_ASSERT.h"
#include "scene/gameoverscene.h"

// include allegro
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
// include scene and following component
#include "scene/sceneManager.h"
#include "board/hexboard.h"
#include "board/movable.h"
#include "monster/monster.h"
#include "system/turn_system.h"
#include "player/player.h"
#include  "ui/damagetext.h"
#include "ui/pebble.h"
#include "ui/status.h"
#include"ui/flash.h"
#include <stdbool.h>
#include "skill/skill.h"
#include "ui/scoretext.h"
#include"ui/bubble.h"

#include"ui/cookie_rain.h"
#include"ui/star_explosion.h"
#include"ui/sugar_dust.h"
#include"ui/peanut_bomb.h"
#include"ui/candy_drop.h"
#include"ui/healtext.h"

#include "monster/gloopie.h"

#include "scene/startscene.h"

// GameWindow.c 或 global.h
//#define DEMO_MODE   



#ifdef DEMO_MODE
static const int demo_skill_map[] = {
    SKL_STRAWBERRY,      // 1
    SKL_MINT_BUBBLE,     // 2
    SKL_CREAM_COOKIE,    // 3
    SKL_MATCHA_CORE,     // 4
    SKL_NOUGAT_BOMB,     // 5
    SKL_CARAMEL_SHROOM,  // 6
    SKL_SUGAR_RUSH,      // 7
    SKL_CREAM_LIFESTEAL  // = 
};
#define DEMO_KEY_COUNT (sizeof(demo_skill_map)/sizeof(demo_skill_map[0]))
#endif



ALLEGRO_SAMPLE *bgm_sample = NULL;
ALLEGRO_SAMPLE_INSTANCE *bgm_instance = NULL;
ALLEGRO_SAMPLE *jump_sample = NULL;
ALLEGRO_SAMPLE *attack_sample = NULL;

static void clear_skill_effects(void) {
    player.damage_multiplier     = 1.0f;  // Sugar Rush
    player.damage_multiplier2    = 1.0f;  // Snow Frosting
    player.lifesteal             = 0.0f;  // Cream Lifesteal
    player.counter_multiplier    = 0.0f;  // Sweet Counter
    player.damage_reduction      = 0.0f;  // Cream Shield

}
static void switch_scene(SceneType next_window)
{
    if (scene) {
        scene->Destroy(scene);
        scene = NULL;
    }
    window = next_window;
    create_scene(next_window);
}

static void handle_game_end(bool won)
{
    game_won = won;
    final_score = score;
    switch_scene(GameOver_L);
}
ALLEGRO_BITMAP* background_bitmap = NULL;
Game *New_Game()
{
    Game *game = (Game *)malloc(sizeof(Game));
    game->execute = execute;
    game->game_init = game_init;
    game->game_update = game_update;
    game->game_draw = game_draw;
    game->game_destroy = game_destroy;
    game->title = "Dessert Hop!";
    game->display = NULL;
    game->game_init(game);
    return game;
}
void execute(Game *self)
{
    // main game loop
    bool run = true;
    while (run)
    {
        // process all events here
        al_wait_for_event(event_queue, &event);
        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
        {
            run &= self->game_update(self);
            self->game_draw(self);
            break;
        }
        case ALLEGRO_EVENT_DISPLAY_CLOSE: // stop game
        {
            run = false;
            break;
        }
        case ALLEGRO_EVENT_KEY_DOWN:
        {
            printf("KEY_DOWN event received: %d\n", event.keyboard.keycode);
            key_state[event.keyboard.keycode] = true;
            /* === 新增：Backspace 或 Delete 悔一步 === */
        if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE ||
            event.keyboard.keycode == ALLEGRO_KEY_DELETE)
        {
            if (!auto_moving && undo_last_step()) {
                printf("Undo to (%d,%d)\n", player.r, player.c);
            }
        }
            
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                printf("ENTER pressed\n"); 
                toggle_skill_scene(); // 切換技能畫面顯示狀態
            }


            #ifdef DEMO_MODE
    {
        int demo_index = -1;
        int k = event.keyboard.keycode;
        if      (k >= ALLEGRO_KEY_1 && k <= ALLEGRO_KEY_7)
            demo_index = k - ALLEGRO_KEY_1;   // 0…6
        else if (k == ALLEGRO_KEY_EQUALS)
            demo_index = 7;                   // demo_skill_map[7]

        if (demo_index >= 0 && demo_index < DEMO_KEY_COUNT) {
                    selected_count = 0;

            clear_skill_effects();

            int id = demo_skill_map[demo_index];
            if (selected_count < 1) selected_count = 1;
            selected_skills[0].skill = &all_skills[id];
            selected_skills[0].level = 1;  

             if (demo_index == 0) {  
                player.r = 5;
            player.c = 5;
            reset_walked_paths();  
            update_movable_tiles();
                static const Pos test_spots[6] = {
        {5, 7},
        {4, 8},
        {5, 0},
        {4, 5},
        {6, 2},
        {7, 3}
    };

    for (int i = 0; i < MAX_MONSTERS; i++) {
        monsters[i].alive = false;
    }
    monster_count = 0;


    for (int i = 0; i < 6 && i < MAX_MONSTERS; i++) {

        Monster m = create_monster(MON_SHROOM,
                                   test_spots[i].r,
                                   test_spots[i].c);
        monsters[i] = m;        
        monsters[i].alive = true;
        monster_count++;
        printf("[TEST] Spawn Shroom at (%d,%d)\n",
               test_spots[i].r, test_spots[i].c);
    }
            }
            draw_skill_buttons();
        }
    }
#endif

            
            break;
        }
        case ALLEGRO_EVENT_KEY_UP:
        {
            key_state[event.keyboard.keycode] = false;
            break;
        }
        case ALLEGRO_EVENT_MOUSE_AXES:
        {
            mouse.x = event.mouse.x;
            mouse.y = event.mouse.y;
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
        {
            mouse_state[event.mouse.button] = true;

            if (event.mouse.button == 1) {
                if (is_skill_scene_visible()) {//看得見技能畫面
                    handle_skill_click(event.mouse.x, event.mouse.y);//選技能
                } 
                else {//看不見技能畫面
                    move_player_to_mouse(event.mouse.x, event.mouse.y);//玩家移動到點擊處
                }
            }
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
        {
            mouse_state[event.mouse.button] = false;
            break;
        }
        default:
            break;
        }
    }
}
void game_init(Game *self)
{
    printf("Game Initializing...\n");

    GAME_ASSERT(al_init(), "failed to initialize allegro.");
    GAME_ASSERT(al_install_keyboard(), "failed to install keyboard.");
    GAME_ASSERT(al_install_mouse(), "failed to install mouse.");
    GAME_ASSERT(al_install_audio(), "failed to install audio.");

    GAME_ASSERT(al_init_font_addon(), "failed to init font addon.");
    GAME_ASSERT(al_init_ttf_addon(), "failed to init ttf addon.");
    GAME_ASSERT(al_init_image_addon(), "failed to init image addon.");
    GAME_ASSERT(al_init_acodec_addon(), "failed to init acodec addon.");
    GAME_ASSERT(al_init_primitives_addon(), "failed to init primitives.");
    GAME_ASSERT(al_reserve_samples(16), "failed to reserve samples.");

    printf("Allegro addons installed.\n");

    self->display = al_create_display(WIDTH, HEIGHT);
    GAME_ASSERT(self->display, "failed to create display.");

    event_queue = al_create_event_queue();
    GAME_ASSERT(event_queue, "failed to create event queue.");

    fps = al_create_timer(1.0 / FPS);
    GAME_ASSERT(fps, "failed to create timer");

    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    al_register_event_source(event_queue, al_get_display_event_source(self->display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(fps));

    al_start_timer(fps);

    ALLEGRO_BITMAP *icon = al_load_bitmap("assets/image/icon.png");
    if (icon) {
        al_set_display_icon(self->display, icon);
        al_destroy_bitmap(icon);
    }

    al_set_window_title(self->display, self->title);

    init_player_status_ui();

    printf("Event sources registered.\n");

    window = Start_L;
    create_scene(Start_L);

    background_bitmap = al_load_bitmap("assets/image/background.png");
    if (!background_bitmap) {
        fprintf(stderr, "Failed to load background.png\n");
        exit(1);
    }

    init_monster_graphics();
    init_board();
    init_sugar_dust();
    init_player();
    init_bubbles();
    init_star_explosion();
    init_peanut_bombs();
    init_cookie_rain();
    init_candy_drop();
    init_heal_text();
    init_player_graphics();
    init_skill_system();
    init_gloopie();

    spawn_monsters_if_needed(player.r, player.c);

    bgm_sample = al_load_sample("assets/sound/background.mp3");
    if (!bgm_sample) {
        fprintf(stderr, "Failed to load background music.\n");
        exit(1);
    }

    bgm_instance = al_create_sample_instance(bgm_sample);
    al_attach_sample_instance_to_mixer(bgm_instance, al_get_default_mixer());
    al_set_sample_instance_playmode(bgm_instance, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(bgm_instance, 1.0f);
    al_play_sample_instance(bgm_instance);

    jump_sample = al_load_sample("assets/sound/jump.mp3");
    if (!jump_sample) {
        fprintf(stderr, "Failed to load jump sound.\n");
        exit(1);
    }

    printf("Game initialized successfully.\n");
}
bool game_update(Game *self)
{
    if (exit_requested) {
        return false;
    }

    if (scene) {
        scene->Update(scene);
    }

    if (!game_paused && window == GameScene_L) {

        if (!auto_moving) {
            update_movable_tiles();
        }

        if (auto_moving) {
            player_update();

            /* 
               建議把 next_turn() 放在「自動移動真正完成」的地方。
               如果妳的 player_update() 已經會在走完路徑後把 auto_moving 設成 false，
               那就可以在這裡補判斷。
            */
            if (!auto_moving) {
                next_turn();
            }
        }

        update_monsters_animation();
        update_gloopie();
        update_pebbles();
        update_bubbles();
        flash_update();
        update_cookie_rain();
        update_star_explosion();
        update_peanut_bombs();
        update_candy_drop();
        update_heal_text();

        if (key_state[ALLEGRO_KEY_SPACE]) {
            combo_hits = 0;
            combo_bonus = 1.0f;
            auto_moving = true;
            auto_index = 0;
            auto_tick_delay = 0;
            key_state[ALLEGRO_KEY_SPACE] = false;
        }

        if (player.hp <= 0) {
            handle_game_end(false);
            return true;
        }

        if (current_turn >= 50 && player.hp > 0) {
            handle_game_end(true);
            return true;
        }
    }

    if (scene && scene->scene_end) {
        switch (window) {
            case Start_L:
            case Menu_L:
                switch_scene(GameScene_L);
                break;

            case Instruction_L:
            case GameOver_L:
                switch_scene(Start_L);
                break;

            default:
                return false;
        }
    }

    return true;
}

int screen_width;
int screen_height;

void game_draw(Game *self)
{
    al_clear_to_color(al_map_rgb(100, 100, 100));

    if (window != GameScene_L) {
        if (scene) {
            scene->Draw(scene);
        }
        al_flip_display();
        return;
    }

    int bw = al_get_bitmap_width(background_bitmap);
    int bh = al_get_bitmap_height(background_bitmap);

    al_draw_scaled_bitmap(
        background_bitmap,
        0, 0, bw, bh,
        0, 0, WIDTH, HEIGHT,
        0
    );

    draw_board();
    draw_sugar_dust();
    draw_walked_paths();
    draw_pebbles();
    draw_monsters();
    flash_draw();
    draw_player();
    draw_heal_text();
    draw_bubbles();
    draw_cookie_rain();
    draw_peanut_bombs();
    draw_damage_texts();
    draw_player_status();
    draw_candy_drop();
    draw_combo_ui();
    draw_turn();
    draw_score_ui();
    draw_score_texts();
    draw_star_explosion();
    draw_skill_buttons();

    if (is_skill_scene_visible()) {
        draw_skill_scene();
    }

    al_flip_display();
}

void game_destroy(Game *self)
{
    if (bgm_instance) {
        al_stop_sample_instance(bgm_instance);
        al_destroy_sample_instance(bgm_instance);
        bgm_instance = NULL;
    }

    if (bgm_sample) {
        al_destroy_sample(bgm_sample);
        bgm_sample = NULL;
    }

    if (jump_sample) {
        al_destroy_sample(jump_sample);
        jump_sample = NULL;
    }

    if (attack_sample) {
        al_destroy_sample(attack_sample);
        attack_sample = NULL;
    }

    if (background_bitmap) {
        al_destroy_bitmap(background_bitmap);
        background_bitmap = NULL;
    }

    destroy_monster_graphics();
    destroy_gloopie();
    destroy_player_status_ui();
    destroy_player_graphics();

    if (hex_font) {
        al_destroy_font(hex_font);
        hex_font = NULL;
    }

    if (scene) {
        scene->Destroy(scene);
        scene = NULL;
    }

    if (event_queue) {
        al_destroy_event_queue(event_queue);
        event_queue = NULL;
    }

    if (self->display) {
        al_destroy_display(self->display);
        self->display = NULL;
    }

    free(self);
}