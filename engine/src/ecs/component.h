#pragma once

#include <gr-math/gmath.h>

#include <ostream>
#include <istream>

class GameObject;
class Entity;

class Component {
public:
    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Enable() {}

    virtual void Disable() {}

    virtual std::ostream &Serialize(std::ostream &os) { return os; }

    virtual std::istream &Deserialize(std::istream &is) { return is; }

    virtual ~Component() {}
    
    Entity* entity;

    GameObject *getGameObject();
};


