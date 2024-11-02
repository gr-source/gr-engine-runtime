#include "ecs/gameObject.hpp"

#include "script/transform.hpp"

#include "api.hpp"

GameObject *GameObject::Create(const std::string &name, const std::string &tag) {
    return GameObjectCreate(name, tag);
}

GameObject *GameObject::Instantiate(GameObject *target) {
    throw std::runtime_error("Function added in engine.");
}

GameObject *GameObject::Find(const std::string &name) {
    return nullptr;
}

void GameObject::Destroy(GameObject *target) {}

void GameObject::setName(const std::string &name) {
    m_name = name;
}

const std::string &GameObject::getName() const {
    return m_name;
}

const std::string &GameObject::getTag() const {
    return m_tag;
}

void GameObject::setEnable(bool value) {
    m_bEnable = value;
}

void GameObject::setActive(bool value) {
    m_bActive = value;
}

bool GameObject::isActive() const {
    return m_bActive && m_bEnable;
}

Entity *GameObject::GetEntity() {
    return m_entity;
}

GameObject *GameObject::getChild(int idx) {
    return m_transform->getChildren()[idx]->getGameObject();
}

Transform *GameObject::getTransform() {
    return m_transform;
}


