#pragma once

#include <gr-math/gmath.hpp>

class Screen {
public:
    static int width;
    static int height;

    static void SetResolution(int width, int height);

    static const iVector2 GetResolution();
};

