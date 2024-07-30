#pragma once

#include <gr-math/gmath.h>

class FbxVertex {
public:
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int index;

    FbxVertex(const Vector3 & position, const Vector3 & normal, const Vector2 & texCoord, int index) : position(position), normal(normal), texCoord(texCoord), index(index) {}


    // FbxVertex(const Vector3 &position = Vector3::zero, const Vector3 &normal = Vector3::zero, const Vector2 &texCoord = Vector2::zero, int index = 0);
};


