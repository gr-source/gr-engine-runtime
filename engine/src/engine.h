#pragma once

#include "Game.h"

#include <iostream>

class engine {
public:
    static void OnResizeWindow(u32 width, u32 height);

    static void MainLoop(std::function<void()> func);

    static void Escap();

    static void WindowInstance(void* instance);

    static void OnInit();

    static void OnRender();

    static void OnUpdate();

    static void OnPhysics();

    static void OnLoader();

    static void OnDestroy();

protected:
    static void RenderScene();
    
    static void MainFramebuffer();

    #if !IS_GAME
    static void UIFramebuffer();
    #endif
    
    static void SetupEnable();

    static std::unique_ptr<Game> m_game;

    static Skybox* m_skybox;
    
    static std::thread m_thread[2];
};

