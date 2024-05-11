#pragma once

#include "Game.h"

#include <GLFW/glfw3.h>

#include <iostream>

class engine {
public:
    static void GLFWkeyfunCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void GLFWframebuffersizefunCallback(GLFWwindow* window, int width, int height);

    static void GLFWscrollfun(GLFWwindow* window, double xoffset, double yoffset);

    static void GLFWcursorposfun(GLFWwindow* window, double xpos, double ypos);

    static void GLFWcharfun(GLFWwindow* window, unsigned int codepoint);

    static void GLFWmousebuttonfun(GLFWwindow* window, int button, int action, int mods);

    static void GLFWcursorenterfun(GLFWwindow* window, int entered);

    static void GLFWdropfun(GLFWwindow* window, int path_count, const char* paths[]);

    static void OnScreenSize(int width, int height);

    static void OnInit();

    static void OnRender();

    static void OnUpdate();

    static void OnPhysics();

    static void OnLoader();

    static void OnDestroy();

protected:
    static void RenderScene();
    
    static void MainFramebuffer();

    static void RenderLighs();

    static void PosFramebuffer();

    #if !IS_GAME
    static void UIFramebuffer();
    #endif
    
    static void SetupEnable();

    static std::unique_ptr<Game> m_game;

    static Skybox* m_skybox;
};

