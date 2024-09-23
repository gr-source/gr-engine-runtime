#pragma once

#include <gr-math/gmath.h>

class GameObject;
class Entity;

class Component {
public:
    Component();
    
    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Enable() {}

    virtual void Disable() {}

    virtual std::ostream &Serialize(std::ostream &os) { return os; }

    virtual std::istream &Deserialize(std::istream &is) { return is; }

    virtual ~Component() {}
    
    GameObject *getGameObject();

    Entity* entity;
};


#define CLASS_FUNCTIONS \
    template <typename T> \
    inline T &GetComponent() { \
        return entity->getComponent<T>(); \
    } \
    template <typename T> \
    inline bool HasComponent() const { \
        return entity->hasComponent<T>(); \
    } \
    template <typename T> \
    inline T &AddComponent() { \
        return entity->addComponent<T>(); \
    } \
    inline GameObject *GetGameObject() { \
        return entity->getGameObject(); \
    } \


#include "componentRegistry.hpp"

#define REGISTER_COMPONENT(cls) \
    static bool is_register_##cls = (ComponentRegistry::getInstance().RegisterComponent<cls>(), false); \

#define REGISTER_FIELD(cls, type, field) \
    static bool is_register_##cls##_##type##_##field = \
        (ComponentRegistry::getInstance().GetComponentPool<cls>()->RegisterField(#field, #type, reinterpret_cast<std::size_t>(&reinterpret_cast<cls*>(0)->field)), false);


