#pragma once

#include "params.h"

#include <unordered_map>

#include "../types.h"

#include "../engine.h"

using Window = void*;

class Plataform {
public:
    static void Initialize(const GWindow& window);

private:
    static Window m_window;

    static void MainLoop();

    static void Destroy();

    static std::unordered_map<u32, u32> m_keyMap;
};
