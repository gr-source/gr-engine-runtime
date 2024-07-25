#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

std::size_t Entity::GetEntityCount() {
    return ScriptManager::GetEntityCount();
}

GameObject *Entity::getGameObject() {
    return gameObject;
}