#include "component.h"

GameObject * Component::GetGameObject(Component * component) {
    return component->gameObject;
}

