#pragma once

#include "../types.h"

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;

    Vertex(const Vector3& position = Vector3::zero, const Vector3& normal = Vector3::zero, const Vector3& texCoord = Vector3::zero, int mIdx = 0);
};
