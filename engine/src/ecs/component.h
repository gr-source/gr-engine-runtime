#pragma once

class Entity;
class GameObject;

class Component {
public:
    Component(bool active = false) : active(active), entity(nullptr) {}

    Entity* entity;

    GameObject* gameObject;

    bool active;

    virtual void OnInit() {}

    virtual void OnStart() {}

    virtual void OnUpdate() {}

    virtual ~Component() {}
};
