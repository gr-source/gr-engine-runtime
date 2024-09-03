#include "component.h"

#include "entity.h"

Component::Component() : entity(nullptr) {}

GameObject *Component::getGameObject() {
    if (!entity) {
        throw std::runtime_error("Entity is inavlid");
    }
    return entity->getGameObject();
}