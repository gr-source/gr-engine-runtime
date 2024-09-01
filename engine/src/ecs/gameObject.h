#pragma once

#include "default.hpp"

#include "entity.h"


class GameObject {
public:
    static GameObject &(*Create)(const std::string &name, const std::string &tag);

    static void (*Destroy)(GameObject *gameObject);

    static GameObject *(*Find)(const std::string &name);

    static void (*Rename)(GameObject *gameObject, const std::string &name);

    const std::string &getName();

    void setName(const std::string &name);

    const std::string &getTag();

    void setActive(bool value);
    
    bool isActive() const;

    Transform &getTransform();

    template <typename T>
    T &addComponent();

    template <typename T>
    T& getComponent();

    template <typename T>
    bool hasComponent() const;

    template <typename T, typename... Args>
    void RenderProperty(Args&&... args);

    Entity *getEntity();
    
private:
    Transform *m_transform;

    std::string m_name;

    std::string m_tag;

    Entity* m_entity;

    bool m_bActive;
};

template <typename T>
inline T &GameObject::addComponent() {
    return m_entity->addComponent<T>();
}

template <typename T>
inline T &GameObject::getComponent() {
    return m_entity->getComponent<T>();
}

template <typename T>
inline bool GameObject::hasComponent() const {
    return m_entity->hasComponent<T>();
}

template <typename T, typename... Args>
inline void GameObject::RenderProperty(Args&&... args) {
    if (!m_entity->hasComponent<T>()) return;

    std::bind(std::forward<Args>(args)..., &m_entity->getComponent<T>())();
}
