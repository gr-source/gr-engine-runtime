#pragma once

#include "../types.h"

struct Vertex3D {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;

    Vertex3D(const Vector3& position, const Vector3& normal = Vector3::zero, const Vector2& texCoord = Vector2::zero) : position(position), normal(normal), texCoord(texCoord) {}
};
