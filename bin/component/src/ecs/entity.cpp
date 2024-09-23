#include "ecs/entity.hpp"
#include "ecs/gameObject.hpp"

#include "api.hpp"

GameObject *Entity::getGameObject() {
    return m_gameObject;
}

EntityID Entity::getEntityID() const {
    return m_entityID;
}

Component *Entity::addInternalComponent(const std::string &name) {
    return nullptr;
}
