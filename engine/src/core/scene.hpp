#pragma once

#include "../ecs/gameObject.h"

#include <list>

class Scene {
public:
    static std::list<GameObject *> &(*GetGameObject)();
};

