#include "gameObject.h"

#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

GameObject &(*GameObject::Create)(const std::string &name, const std::string &tag);

void (*GameObject::Destroy)(GameObject *gameObject);

GameObject *(*GameObject::Find)(const std::string &name);

void (*GameObject::Rename)(GameObject *gameObject, const std::string &name);

const std::string &GameObject::getName() {
    return m_name;
}

void GameObject::setName(const std::string &name) {
    m_name = name;
}

const std::string &GameObject::getTag() {
    return m_tag;
}

void GameObject::setActive(bool value) {
    m_bActive = value;
}

bool GameObject::isActive() const {
    return m_bActive;
}

Transform &GameObject::getTransform() {
    return *m_transform;
}

Entity *GameObject::getEntity() {
    return m_entity;
}
