#pragma once

#include <gr-math/gmath.hpp>

#include <ostream>
#include <istream>

class GameObject;
class Entity;
class Stream;

#include <ecs/gComponent.hpp>
/*

class Component {
public:
    Component();
    
    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Enable() {}

    virtual void Disable() {}

    virtual bool Serialize(Stream *streaming) { return false; }

    virtual bool Deserialize(Stream *streaming) { return false; }

    virtual ~Component() {}
    
    GameObject *getGameObject();

    Entity* entity;
};
*/

