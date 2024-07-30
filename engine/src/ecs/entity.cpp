#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

void (*Entity::Update)();

GameObject *Entity::getGameObject() {
    return gameObject;
}

std::unordered_map<std::string, std::shared_ptr<Component>> &Entity::getComponents() {
    return components;
}
