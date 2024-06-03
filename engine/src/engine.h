#pragma once

<<<<<<< HEAD
#include <string>

#include "plataform/plataform.h"

class Engine {
public:
    enum Event {
        GR_CREATEWINDOW = 1,
        GR_INITIALIZE   = 2,
        GR_RESIZE       = 3,
        GR_DESTROY      = 10,
        GR_ERROR        = 20,
        GR_UPDATE       = 30,
        GR_RENDERER     = 40,
        GR_KEYBOARD     = 50,
        GR_MOUSE        = 60,
        GR_MOUSE_POS    = 65
    };

    typedef void* (*GRHandleEvent)(Engine::Event, GParams*);

    static bool Initialize(GRHandleEvent handleEnevt);

    static void* SetHandleEvent(Event event, GParams* params = nullptr);

    static void SetError(const std::string& error);

    static const std::string GetError();

private:
    static GRHandleEvent m_handleEvent;

    static std::string m_error;
=======
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
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
};

