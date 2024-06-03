#pragma once

#include <gr-math/gmath.h>

struct BoundBox {
    Vector3 min;
    Vector3 max;

    BoundBox(const Vector3& min = Vector3::zero, const Vector3& max = Vector3::zero) : min(min), max(max) {}
};

