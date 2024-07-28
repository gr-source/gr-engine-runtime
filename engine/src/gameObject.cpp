#include "gameObject.h"

const std::string &GameObject::getName() {
    return m_name;
}

const std::string &GameObject::getTag() {
    return m_tag;
}

Transform &GameObject::getTransform() {
    return m_entity->getComponent<Transform>();
}

Entity *GameObject::getEntity() {
    return m_entity;
}
