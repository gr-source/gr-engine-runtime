#include "gameObject.h"

#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

GameObject *GameObject::Find(const std::string &name) {
    if (!ScriptManager::GetAllGameObject) {
        return nullptr;
    }
    auto& list = ScriptManager::GetAllGameObject();
    
    auto it = std::find_if(list.begin(), list.end(), [&name](const auto& gameObject) {
        return name == gameObject->getName();
    });
    if (it != list.end()) {
        return (*it).get();
    }
    return nullptr;
}

GameObject &(*GameObject::Create)(const std::string &name, const std::string &tag);

const std::string &GameObject::getName() {
    return m_name;
}

void GameObject::setName(const std::string &name) {
    m_name = name;
}

const std::string &GameObject::getTag() {
    return m_tag;
}

Transform &GameObject::getTransform() {
    return *m_transform;
}

Entity *GameObject::getEntity() {
    return m_entity;
}
