#include "component.h"

#include "entity.h"

GameObject * Component::getGameObject() {
    return entity->getGameObject();
}