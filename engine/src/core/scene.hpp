#pragma once

#include "../ecs/gameObject.h"

#include <list>

class Scene {
public:
    static void Remove(GameObject *target);

    static void Add(GameObject *target);
    
    static std::list<GameObject *> &(*GetGameObject)();
};

