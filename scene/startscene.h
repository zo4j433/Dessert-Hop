#ifndef STARTSCENE_H_INCLUDED
#define STARTSCENE_H_INCLUDED

#include "scene.h"
#include "sceneManager.h"   // 取得 SceneType 定義

// 建立 StartScene 的工廠函式
Scene* New_StartScene(SceneType type);

#endif
