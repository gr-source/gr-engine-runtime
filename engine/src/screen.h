#pragma once

#include <gr-math/vector2.h>

class Screen {
public:
    static float width;
    static float height;

    static void SetResolution(float width, float height);

    static const Vector2 GetResolution();
};

