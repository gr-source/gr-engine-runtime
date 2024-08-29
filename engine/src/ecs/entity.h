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

    Component *addComponent(const std::string &name);

    template <typename T>
    T& getComponent();
    
    template <typename T>
    const bool hasComponent() const;

    GameObject *getGameObject();

    std::unordered_map<std::string, std::shared_ptr<Component>> &getComponents();
    
private:
    std::unordered_map<std::string, std::shared_ptr<Component>> m_componentPtr;

    std::vector<Component *> m_components;

    GameObject *gameObject;

    grm::u32 m_entityID;

    Component *addComponentInternal(const std::string &name);
};

template <typename T, typename... Args>
inline T &Entity::addComponent(Args &&...args) {
    auto name = typeid(T).name();

    auto &&[iterator, added] = m_componentPtr.emplace(name, std::make_shared<T>(std::forward<Args>(args)...));

    auto& component = (*iterator).second;
    component->entity = this;
    component->Init();

    m_components.push_back(&(*component));

    return *dynamic_cast<T *>(&(*component));
}

template <typename T>
inline T &Entity::getComponent() {
    return *dynamic_cast<T*>(m_componentPtr[typeid(T).name()].get());
}

template <typename T>
inline const bool Entity::hasComponent() const {
    auto it = m_componentPtr.find(typeid(T).name());
    return it != m_componentPtr.end();
}
