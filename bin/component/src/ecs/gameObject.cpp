#include "ecs/gameObject.hpp"

#include "script/transform.hpp"

#include "core/function.hpp"

GameObject *(*GameObject::Create)(const std::string &name, const std::string &tag);
INSERT_FUNCTION(0, GameObject::Create)

GameObject *(*GameObject::Instantiate)(GameObject *target);
INSERT_FUNCTION(1, GameObject::Instantiate)

GameObject *(*GameObject::Find)(const std::string &name);
INSERT_FUNCTION(2, GameObject::Find)

void (*GameObject::Destroy)(GameObject *target);
INSERT_FUNCTION(3, GameObject::Destroy)

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


