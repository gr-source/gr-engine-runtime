#pragma once

#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <vector>
#include <bitset>
#include <array>

#include "component.h"

class GameObject;

/* tamanho maximo de componenets */
constexpr std::size_t maxComponent = 32;

/* gere um id para */
inline std::size_t GenerateID() noexcept {
    static std::size_t id = 0;
    return ++id;
}

/* Gere um id apartir de um componenet */
template <typename T>
inline std::size_t GetIDFromComponent() noexcept {
    static std::size_t id = GenerateID();
    return id;
}

/* Classe de entidades */
class Entity {
public:
    std::unordered_map<std::size_t, std::shared_ptr<Component>> components;
    std::bitset<32> m_bitset;
    bool IsActivate;

public:
    GameObject* m_gameobject;

    Entity(GameObject* gameobject);

    template <typename T, typename... Args>
    T& addComponent(Args&& ... args);

    template <typename T>
    T& getComponent();

    template <typename T>
    const bool hasComponent() const;

    void OnStart();

    void OnUpdate();

    bool operator==(const Entity& other) const;
};

template <typename T, typename... Args>
inline T& Entity::addComponent(Args&&... args) {
    if (hasComponent<T>()) {
        return getComponent<T>();
    }
    
    components.emplace(GetIDFromComponent<T>(), std::make_shared<T>(std::forward<Args>(args)...));

    auto& c = components[GetIDFromComponent<T>()];
    c->gameObject = m_gameobject;
    c->entity = this;

    m_bitset.set(GetIDFromComponent<T>(), true);

    c->OnInit();

    #if IS_GAME
    c->OnStart();
    #endif

    return *dynamic_cast<T*>(c.get());
}

template <typename T>
inline T& Entity::getComponent() {
    if (m_bitset.test(GetIDFromComponent<T>())) {
        return *dynamic_cast<T*>(components[GetIDFromComponent<T>()].get());
    }
    throw std::runtime_error("Component not exists!");
}

template <typename T>
inline const bool Entity::hasComponent() const {
    return m_bitset.test(GetIDFromComponent<T>());
}



