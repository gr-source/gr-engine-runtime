#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

void (*Entity::Update)();

GameObject *Entity::getGameObject() {
    return gameObject;
}

std::unordered_map<std::string, std::shared_ptr<Component>> &Entity::getComponents() {
    return m_componentPtr;
}

Component *Entity::addInternalComponent(const std::string &name) {
    auto component = static_cast<Component *>(ScriptManager::CreateComponentWithName(name));
    component->entity = this;
    component->Init();

    m_componentPtr.emplace(name, std::shared_ptr<Component>(component));

    m_components.push_back(component);

    return component;
}
