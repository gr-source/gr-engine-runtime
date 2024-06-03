#pragma once

#include "../common.h"

struct GWindow {
    std::string title;
    u32 width;
    u32 height;
    u32 x;
    u32 y;
};

struct GKeyboard {
    u32 key;
    u32 action;
};

struct GMouse {
    float x;
    float y;
};

struct GParams {
    GWindow window;
    GKeyboard keyboard;
    GMouse mouse;
};

