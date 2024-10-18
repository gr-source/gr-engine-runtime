#pragma once

#include <gr-math/gmath.h>

#include <ostream>
#include <istream>

class GameObject;
class Entity;
class Streaming;

class Component {
public:
    Component();
    
    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Enable() {}

    virtual void Disable() {}

    virtual bool Serialize(Streaming *streaming) { return false; }

    virtual bool Deserialize(Streaming *streaming) { return false; }

    virtual ~Component() {}
    
    GameObject *getGameObject();

    Entity* entity;
};


