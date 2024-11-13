#pragma once

#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <memory>
#include <vector>
#include <bitset>
#include <array>

#include "default.hpp"
#include "component.hpp"

#define MAX_COMPONENT 16
#define MAX_PAGES 10

class Entity {
public:
    Entity() = delete;
    Entity(GameObject *gameObject, EntityID id);

    template <typename T>
    T *addComponent();

    template <typename T>
    T *getComponent();
    
    template <typename T>
    bool hasComponent() const;

    Component *addComponent(const std::string &name, const Component *instance = nullptr);

    Component *getComponent(const std::string &name);
    
    bool hasComponent(const std::string &name) const;

    Component *addInternalComponent(const std::string &name, const Component *instance = nullptr);
    
    GameObject *getGameObject();

    std::unordered_map<std::string, std::unique_ptr<Component>> &getComponents();

    void copy(Entity *entity);

    EntityID getEntityID() const;

    bool isActive() const;

    void setEnable(bool value);

    void update();

    void init();

    void clear();

    void remove(const std::string &name);
    
private:
    std::unordered_map<std::string, std::unique_ptr<Component>> m_componentPtr;

    std::vector<Component *> m_components;

    GameObject *m_gameObject;

    EntityID m_entityID;
};

#include "core/string.hpp"

template <typename T>
inline T *Entity::addComponent() {
    auto component = addInternalComponent(String::RemoveNumber(typeid(T).name()));
    if (!component) {
        return nullptr;
    }
    return dynamic_cast<T *>(component);
}

template <typename T>
inline T *Entity::getComponent() {
    auto typeName = String::RemoveNumber(typeid(T).name());
    
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
    return m_componentPtr.find(String::RemoveNumber(typeid(T).name())) != m_componentPtr.end();
}
