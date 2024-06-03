#pragma once

<<<<<<< HEAD
#include "types.h"

class Screen {
public:
    static u32 width;
    static u32 height;

    static void SetResolution(u32 width, u32 height);
=======
#include <gr-math/vector2.h>

class Screen {
public:
    static float width;
    static float height;

    static void SetResolution(float width, float height);
>>>>>>> 7c9a1b2 (	new file:   .gitignore)

    static const Vector2 GetResolution();
};

