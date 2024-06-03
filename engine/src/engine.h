#pragma once

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
};

