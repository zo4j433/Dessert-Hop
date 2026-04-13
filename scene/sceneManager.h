#ifndef SCENEMANAGER_H_INCLUDED
#define SCENEMANAGER_H_INCLUDED

#include "scene.h"
extern Scene *scene;

typedef enum SceneType
{
    Start_L = 0,
     Instruction_L,    // 新增開始場景
    Menu_L,
    GameScene_L,
     GameOver_L
} SceneType;

void create_scene(SceneType);

#endif
