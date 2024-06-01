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
    static void Initialize(Monitor window);
private:
    static Window m_window;

    static void MainLoop();

    static void Destroy();
};
