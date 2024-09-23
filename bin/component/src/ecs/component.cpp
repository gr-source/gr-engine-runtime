#include "ecs/component.hpp"
#include "ecs/entity.hpp"

Component::Component() : entity(nullptr) {}

GameObject *Component::getGameObject() {
    return entity->getGameObject();
}



