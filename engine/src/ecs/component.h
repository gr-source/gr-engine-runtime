#pragma once

#include <gr-math/gmath.h>

class GameObject;
class Entity;

class Component {
public:
    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual ~Component() {}

    GameObject *getGameObject();

    Entity* entity;
};
