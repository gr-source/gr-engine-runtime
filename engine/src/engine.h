#pragma once

#include <string>

#include "plataform/plataform.h"

class Engine {
public:
    enum Event {
        GRINITIALIZE = 1,
        GRDESTROY = 2,
        GRERROR = 3,
        GRUPDATE = 4,
        GRRENDERER = 5
    };

    typedef void (*GRHandleEvent)(Engine::Event event);

    static bool Initialize(GRHandleEvent handleEnevt);

    static void SetHandleEvent(Event event);

    static void SetError(const std::string& error);

    static const std::string GetError();

    static Monitor monitor;

private:
    static GRHandleEvent m_handleEvent;

    static std::string m_error;
};

