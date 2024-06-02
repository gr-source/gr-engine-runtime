#pragma once

#include "types.h"

class Screen {
public:
    static u32 width;
    static u32 height;

    static void SetResolution(u32 width, u32 height);

    static const Vector2 GetResolution();
};

