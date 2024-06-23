#pragma once

#include <utility>
#include <string>

#include "ecs/entity.h"

#include "component/Transform.h"

class GameObject {
public:

    GameObject(const std::string& name = "none", const std::string& tag = "none");
    ~GameObject();

    const std::string& getName() const;

    const std::string& getTag() const;

    void setActive(bool active);

    const bool &getActive() const;

    /* adiciono um componenet ao objeto */
    template <typename T, typename ...Argv>
    constexpr T& addComponent(Argv&&... argv);

    /* pegue um component do objeto */
    template <typename T>
    T& getComponent();

    /* verifique de o component existe nesse objeto */
    template <typename T>
    constexpr bool hasComponent() const;

    Transform& getTransform();

    template <typename T, typename... Args>
    void RenderProperty(Args&&... args);

    bool operator ==(const GameObject& other) const;
private:
    Entity* m_entity;

    std::string m_name;

    std::string m_tag;

    bool m_active;
};

template <typename T, typename ...Argv>
inline constexpr T& GameObject::addComponent(Argv&&... argv) {
    return m_entity->addComponent<T>(std::forward<Argv>(argv)...);
}

template <typename T>
inline T& GameObject::getComponent() {
    return m_entity->getComponent<T>();
}

template <typename T>
inline constexpr bool GameObject::hasComponent() const {
    return m_entity->hasComponent<T>();
}

template <typename T, typename... Args>
inline void GameObject::RenderProperty(Args&&... args) {
    if (!m_entity->hasComponent<T>()) return;

    std::bind(std::forward<Args>(args)..., &m_entity->getComponent<T>())();
}
