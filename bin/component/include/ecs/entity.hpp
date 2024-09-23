#pragma once

#include <unordered_map>
#include <bitset>
#include <cstdio>
#include <vector>
#include <memory>
#include <list>

#include "component.hpp"

#define MAX_COMPONENT 16
#define MAX_PAGES 10

using ComponentID = grm::size;
using EntityID = grm::size;

using ComponenetArray = std::array<Component *, MAX_COMPONENT>;
using ComponenetMask = std::bitset<MAX_COMPONENT>;

class Entity {
public:
    Entity() = delete;
    ~Entity() = delete;

    template <typename T>
    T &addComponent();

    template <typename T>
    T &getComponent();
    
    template <typename T>
    bool hasComponent() const;

    GameObject *getGameObject();

    EntityID getEntityID() const;

    Component *addInternalComponent(const std::string &name);
    
private:
    std::unordered_map<std::string, Component *> m_componentPtr;

    std::vector<std::unique_ptr<Component>> m_components;

    GameObject *m_gameObject;

    EntityID m_entityID;
};

template <typename T>
inline T &Entity::addComponent() {
    auto component = addInternalComponent(StringUTF::RemoveNumber(typeid(T).name()));
    if (!component) {
        throw std::runtime_error("Component is null.");
    }
    
    auto typedComponent = dynamic_cast<T *>(component);
    if (!typedComponent) {
        throw std::runtime_error("Component cast failed.");
    }

    return *typedComponent;
}

template <typename T>
inline T &Entity::getComponent() {
    auto typeName = StringUTF::RemoveNumber(typeid(T).name());
    
    auto it = m_componentPtr.find(typeName);
    if (it == m_componentPtr.end()) {
        throw std::runtime_error("Component type not found.");
    }

    auto component = it->second;
    if (!component) {
        throw std::runtime_error("Component pointer is null.");
    }

    auto typedComponent = dynamic_cast<T *>(component);
    if (!typedComponent) {
        throw std::runtime_error("Component cast failed.");
    }

    return *typedComponent;
}

template <typename T>
inline bool Entity::hasComponent() const {
    return m_componentPtr.find(StringUTF::RemoveNumber(typeid(T).name())) != m_componentPtr.end();
}
