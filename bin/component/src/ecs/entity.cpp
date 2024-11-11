#include "ecs/entity.hpp"
#include "ecs/gameObject.hpp"

#include "core/function.hpp"

Component *(*Entity::addInternalComponent)(Entity *instance, const std::string &name, const Component *component);
INSERT_FUNCTION(0, Entity::addInternalComponent)

GameObject *Entity::getGameObject() {
    return m_gameObject;
}

EntityID Entity::getEntityID() const {
    return m_entityID;
}




