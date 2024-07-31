#pragma once

#include <string>

#include "entity.h"

#include "../component/transform.hpp"

class GameObject {
public:
    const std::string &getName();

    const std::string &getTag();

    Transform &getTransform();

    template <typename T>
    T& getComponent();

    template <typename T, typename... Args>
    void RenderProperty(Args&&... args);

    Entity *getEntity();
    
private:
    std::string m_name;

    std::string m_tag;

    Entity* m_entity;

protected:
    static std::vector<std::unique_ptr<GameObject>> s_objects;
};

template <typename T>
inline T& GameObject::getComponent() {
    return m_entity->getComponent<T>();
}

template <typename T, typename... Args>
inline void GameObject::RenderProperty(Args&&... args) {
    if (!m_entity->hasComponent<T>()) return;

    std::bind(std::forward<Args>(args)..., &m_entity->getComponent<T>())();
}
