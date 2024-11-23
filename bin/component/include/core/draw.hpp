#pragma once

#include <gr-math/gmath.hpp>

class Draw {
public:
    static void Line(const Vector3 &start, const Vector3 &end, const Color &color);

    static void (*CallInternalDrawLine)(const Vector3 &start, const Vector3 &end, const Color &color);
};



