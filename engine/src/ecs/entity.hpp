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
    ~Entity();

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
    
    GameObject *getGameObject() const;

    std::unordered_map<std::string, Component *> &getComponents();

    void copy(const Entity *entity);

    EntityID getEntityID() const;

    bool isActive() const;

    void setEnable(bool value);

    void init();
    
    void update();

    void destroy();

    void clear();

    void remove(const std::string &name);
    
    void reintantiate();
    
private:
    std::unordered_map<std::string, Component *> m_componentPtr;

    std::vector<Component *> m_components;

    GameObject *m_gameObject;

    EntityID m_entityID;
};

#include "core/string.hpp"

template <typename T>
inline T *Entity::addComponent() {
    static_assert(sizeof(T) != 0, "Type component imcompleted.");
    
    auto component = addInternalComponent(String::RemoveNumber(typeid(T).name()));
    if (!component) {
        return nullptr;
    }
    return dynamic_cast<T *>(component);
}

// TODO: Troquei dinamic_cast para static_cast. verificar se nao vai da erro
template <typename T>
inline T *Entity::getComponent() {
    static_assert(sizeof(T) != 0, "Type component imcompleted.");
    
    auto typeName = String::RemoveNumber(typeid(T).name());
    
    auto it = m_componentPtr.find(typeName);
    if (it == m_componentPtr.end()) {
        throw std::runtime_error("Component type not found.");
    }

    auto component = &(*it->second);
    if (!component) {
        throw std::runtime_error("Component pointer is null.");
    }

    auto typedComponent = static_cast<T *>(component);
    if (!typedComponent) {
        throw std::runtime_error("Component cast failed.");
    }

    return typedComponent;
}

template <typename T>
inline bool Entity::hasComponent() const {
    return m_componentPtr.find(String::RemoveNumber(typeid(T).name())) != m_componentPtr.end();
}
