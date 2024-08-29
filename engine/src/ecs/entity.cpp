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

Component *Entity::addComponent(const std::string &name) {
    auto ptr = addComponentInternal(name);

    auto &&[iterator, added] = m_componentPtr.emplace(name, std::shared_ptr<Component>(ptr));

    auto& component = (*iterator).second;
    component->entity = this;
    component->Init();

    m_components.push_back(&(*component));

    return ptr;
}

Component *Entity::addComponentInternal(const std::string &name) {
    return static_cast<Component *>(ScriptManager::CreateComponentWithName(name));
}
