#pragma once

#include "ecs/default.hpp"

class Hit {
public:
    GameObject *collisionObject;

    Vector3 hitNormal;

    Vector3 hitPoint;
};

class Raycast {
public:
    static bool (*RayCast)(const Vector3 &rayStart, const Vector3 &rayEnd, Hit &hit);
};



