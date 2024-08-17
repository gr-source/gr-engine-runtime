#pragma once

#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <vector>
#include <bitset>
#include <array>

#include "component.h"

#define MAX_COMPONENT 16

using ComponentID = std::size_t;

class GameObject;

class ComponentIDGenerator {
public:
    template <typename T>
    static ComponentID GenerateIDByType() noexcept {
        static ComponentID id = GenerateID();
        return id;
    }

private:
    static ComponentID GenerateID() noexcept {
        static ComponentID id = 0;
        return id++;
    }
};

/* Classe de entidades */
class Entity {
public:
    static void (*Update)();

    template <typename T, typename... Args>
    T &addComponent(Args &&...args);

    template <typename T>
    T& getComponent();
    
    template <typename T>
    const bool hasComponent() const;

    GameObject *getGameObject();

    std::unordered_map<std::string, std::shared_ptr<Component>> &getComponents();
    
private:
    std::unordered_map<std::string, std::shared_ptr<Component>> components;

    GameObject *gameObject;
};

template <typename T, typename... Args>
inline T &Entity::addComponent(Args &&...args) {
    if (hasComponent<T>()) {
        return getComponent<T>();
    }
    auto name = typeid(T).name();

    auto &&[iterator, added] = components.emplace(name, std::make_shared<T>(std::forward<Args>(args)...));

    auto& component = (*iterator).second;
    component->entity = this;
    component->Init();

    return *dynamic_cast<T*>(component.get());
}

template <typename T>
inline T &Entity::getComponent() {
    return *dynamic_cast<T*>(components[typeid(T).name()].get());
}

template <typename T>
inline const bool Entity::hasComponent() const {
    auto it = components.find(typeid(T).name());
    return it != components.end();
}

