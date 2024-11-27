#pragma once

#include <gr-math/gmath.hpp>

class GameObject;
class Stream;
class Entity;

#include <ecs/gComponent.hpp>

#define CLASS_FUNCTIONS \
    template <typename T> \
    inline T *AddComponent() { \
        return entity->addComponent<T>(); \
    } \
    template <typename T> \
    inline T *GetComponent() { \
        return entity->getComponent<T>(); \
    } \
    template <typename T> \
    inline bool HasComponent() const { \
        return entity->hasComponent<T>(); \
    } \
    inline GameObject *GetGameObject() { \
        return entity->getGameObject(); \
    } \


#include <ecs/componentRegistry.hpp>

#include "core/string.hpp"

#define REGISTER_COMPONENT(cls) \
    static bool is_register_##cls = (ComponentRegistry::getInstance().RegisterComponent<cls>(), false); \

#define REGISTER_FIELD(cls, type, field) \
    static bool is_register_##cls##_##type##_##field = \
        (ComponentRegistry::getInstance().GetComponentPool<cls>()->RegisterField(#field, FieldType_ ##type, reinterpret_cast<std::size_t>(&reinterpret_cast<cls *>(0)->field)), false);


