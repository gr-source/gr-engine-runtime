#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

std::size_t Entity::GetEntityCount() {
    return ScriptManager::GetEntityCount();
}

void Entity::Process() {
    if (!ScriptManager::EntityProcess) {
        return;
    }
    return ScriptManager::EntityProcess();
}

GameObject *Entity::getGameObject() {
    return gameObject;
}