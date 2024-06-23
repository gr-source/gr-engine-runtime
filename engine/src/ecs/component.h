#pragma once

class Entity;
class GameObject;

#include <gr-math/gmath.h>

class Component {
public:
    Component(bool active = false) : active(active), entity(nullptr) {}

    Entity* entity;

    GameObject* gameObject;

    bool active;

    // É chamado quando o GameObject' for criado apenas 1 vez.
    virtual void OnInit() {}

    // É chamado quando o game object e instanciado na cena 1 vez.
    virtual void OnStart() {}

    // É chamado frame a frame
    virtual void OnUpdate() {}

    // É chamado quando o game object for destruido
    virtual ~Component() {}


    static GameObject* GetGameObject(Component* component);
};
