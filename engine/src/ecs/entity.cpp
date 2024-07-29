#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

void (*Entity::Update)();

std::size_t Entity::GetEntityCount() {
    return ScriptManager::GetEntityCount();
}

void Entity::RemoveComponent(const std::string &name) {
    auto it = components.find(name);
    if (it != components.end()) {
        components.erase(it);
    }
}

GameObject *Entity::getGameObject() {
    return gameObject;
}

std::unordered_map<std::string, std::shared_ptr<Component>> &Entity::getComponents() {
    return components;
}
