#pragma once

#include "Game.h"

#include <GLFW/glfw3.h>

#include <iostream>

class engine {
public:
    static void GLFWkeyfunCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void GLFWframebuffersizefunCallback(GLFWwindow* window, int width, int height);

    static void GLFWdropfun(GLFWwindow* window, int path_count, const char* paths[]);

    static void OnScreenSize(int width, int height);

    static void MainLoop();

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

