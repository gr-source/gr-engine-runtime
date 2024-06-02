#pragma once

#include <string>

#include "plataform/plataform.h"

class Engine {
public:
    enum Event {
        GRINITIALIZE       = 1,
        GRINITIALIZEOPENGL = 2,
        GRRESIZE           = 3,
        GRDESTROY          = 10,
        GRERROR            = 20,
        GRUPDATE           = 30,
        GRRENDERER         = 40
    };

    typedef void (*GRHandleEvent)(Engine::Event event);

    static bool Initialize(GRHandleEvent handleEnevt);

    static void SetHandleEvent(Event event);

    static void SetError(const std::string& error);

    static const std::string GetError();

private:
    static GRHandleEvent m_handleEvent;

    static std::string m_error;
};

