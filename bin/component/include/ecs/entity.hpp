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

    static Component *(*addInternalComponent)(Entity *instance, const std::string &name, const Component *component);

    template <typename T>
    T *addComponent();

    template <typename T>
    T *getComponent();
    
    template <typename T>
    bool hasComponent() const;

    bool hasComponent(const std::string &name) const;

    GameObject *getGameObject();

    EntityID getEntityID() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Component>> m_componentPtr;

    std::vector<Component *> m_components;

    GameObject *m_gameObject;

    EntityID m_entityID;
};

template <typename T>
inline T *Entity::addComponent() {
    static_assert(sizeof(T) != 0, "Type component imcompleted.");
    
    auto component = addInternalComponent(this, StringUTF::RemoveNumber(typeid(T).name()), nullptr);
    if (!component) {
        throw std::runtime_error("Component is null.");
    }
    
    auto typedComponent = dynamic_cast<T *>(component);
    if (!typedComponent) {
        throw std::runtime_error("Component cast failed.");
    }

    return typedComponent;
}

template <typename T>
inline T *Entity::getComponent() {
    static_assert(sizeof(T) != 0, "Type component imcompleted.");
    
    auto typeName = StringUTF::RemoveNumber(typeid(T).name());
    
    auto it = m_componentPtr.find(typeName);
    if (it == m_componentPtr.end()) {
        throw std::runtime_error("Component type not found.");
    }

    auto component = &(*it->second);
    if (!component) {
        throw std::runtime_error("Component pointer is null.");
    }

    auto typedComponent = dynamic_cast<T *>(component);
    if (!typedComponent) {
        throw std::runtime_error("Component cast failed.");
    }

    return typedComponent;
}

template <typename T>
inline bool Entity::hasComponent() const {
    return m_componentPtr.find(StringUTF::RemoveNumber(typeid(T).name())) != m_componentPtr.end();
}
