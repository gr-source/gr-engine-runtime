#pragma once

#include "../types.h"

struct Monitor {
    std::string title;
    u32 width;
    u32 height;
    u32 x;
    u32 y;
};

using Window = void*;

class Plataform {
public:
    static void Initialize();

    static Monitor& GetMonitor();
private:
    static Monitor m_monitor;

    static Window m_window;

    static void MainLoop();

    static void Destroy();
};
