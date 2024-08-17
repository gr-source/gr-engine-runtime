#pragma once

#include "Game.h"

#include <iostream>

class Engine {
public:
    enum EngineEvent {
        GR_INITIALIZE = 1 << 1,
        GR_DESTROY = 1 << 2,
        GR_RESIZE = 1 << 3,
        GR_KEYBOARD_DOWN = 1 << 4,
        GR_KEYBOARD_UP = 1 << 5,
        GR_CURSOR_POS = 1 << 6,
        GR_MAINLOOP = 1 << 7,
        GR_CURSOR_SCROLL = 1 << 8,
        GR_DROFUNC = 1 << 9
    };

    static void HandleEvent(EngineEvent event, const void* data);

    static PostProcessing *ImGuiPostProcess;

    static PostProcessing *MainPostProcess;

protected:
    static void GRWindowResize(u32 width, u32 height);

    static void MainLoop(std::function<void()> func);

    static void GRKeyboardDown(u32 key);
    static void GRKeyboardUp(u32 key);
    static void GRCursorPos(float xpos, float ypos);
    static void GRCursorScrool(float xoffset, float yoffset);
    static void GRDropFunc(const char* path);

    static void OnInit();

    static void OnRender();

    static void OnUpdate();

    static void OnPhysics();

    static void OnLoader();

    static void OnDestroy();

    static void RenderScene();
    
    static void MainFramebuffer();

    #if !IS_GAME
    static void UIFramebuffer();
    #endif
    
    static void SetupEnable();
    
    static std::thread m_thread[2];

    static Skybox* m_skybox;
};

