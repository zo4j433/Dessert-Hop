#include "sceneManager.h"
#include "startscene.h"   // 新增：StartScene 的宣告
#include "menu.h"
#include "gamescene.h"
#include "gameoverscene.h"
#include "instructionscene.h"
Scene *scene = NULL;


void create_scene(SceneType type)
{
    switch (type)
    {
    case Start_L:
        scene = New_StartScene(Start_L);
        break;
    case Menu_L:
        scene = New_Menu(Menu_L);
        break;
    case Instruction_L:
        scene = New_InstructionScene(Instruction_L);
        break;
    case GameScene_L:
        scene = New_GameScene(GameScene_L);
        break;
    case GameOver_L:
        scene = New_GameOverScene(GameOver_L);
        break;
    default:
        scene = NULL;
        break;
    }
}
