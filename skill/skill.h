#ifndef SKILL_H
#define SKILL_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <stdbool.h>
#define MAX_SKILLS 15
#define MAX_SKILL_LEVEL     6
#define MAX_SELECTED_SKILLS 5

void update_skill_selection(void);
//基本常數
#define MAX_SELECTED_SKILLS 5

//技能 ID
typedef enum {
    SKL_STRAWBERRY,      // 草莓爆爆果凍
    SKL_MATCHA_TORNADO,  // 抹茶漩渦蛋糕
    SKL_SESAME_LINE,     // 芝麻脆片連線
    SKL_MINT_BUBBLE,     // 薄荷煙霧泡泡糖
    SKL_CREAM_COOKIE,    // 奶油爆漿餅乾
    SKL_MATCHA_CORE,     // 抹茶奶油爆心
    SKL_NOUGAT_BOMB,     // 花生牛軋糖碎片
    SKL_CARAMEL_SHROOM,  // 焦糖蘑菇糖球
    SKL_SUGAR_DUST,      // 糖粉灑落
    SKL_SUGAR_RUSH,      // 糖分暴衝
    SKL_SNOW_SALT,       // 雪鹽糖霜倍擊
    SKL_CREAM_LIFESTEAL, // 奶油回血
    SKL_SWEET_COUNTER,   // 甜甜反擊
    SKL_CREAM_SHIELD,    // 奶油保護膜
    SKL_FRUIT_HEAL,      // 鮮果補血露
    SKL_COUNT            // 技能總數
} SkillID;


typedef struct Skill Skill; 

typedef void (*JumpLandCB) (Skill *self, int r, int c);
typedef void (*TurnStartCB)(Skill *self);                
typedef void (*TurnEndCB)(struct Skill *self);


struct Skill {
    int           id;
    const char   *name;
    ALLEGRO_COLOR color;
    int           level;

    JumpLandCB    on_jump_land;
    TurnStartCB   on_turn_start;
    TurnEndCB     on_turn_end;
     bool        available;
};


typedef struct {
    Skill *skill;
    int    level;
} SelectedSkill;


extern SelectedSkill selected_skills[MAX_SELECTED_SKILLS];
extern int           selected_count;

extern bool          show_skill_scene;
extern ALLEGRO_FONT *skill_font;

extern Skill         all_skills[SKL_COUNT];


void init_skill_system      (void);
void toggle_skill_scene     (void);
bool is_skill_scene_visible (void);
void draw_skill_scene       (void);
void draw_skill_buttons     (void);
void handle_skill_click     (float mx, float my);
bool select_skill           (int index);

//事件派發器 (遊戲主程式會呼叫)
void trigger_on_jump_land   (int r, int c);
void trigger_on_turn_start  (void);
void trigger_on_turn_end(void);

#endif
