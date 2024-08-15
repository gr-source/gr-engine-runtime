#pragma once

#include "../ecs/gameObject.h"

#include <list>

class Scene {
public:
    static std::list<GameObject *> &(*GetGameObject)();

    static void (*Delete)(GameObject *target);
    
    static GameObject *(*Create)(const std::string &);

    static void Remove(GameObject *target);

    static void Add(GameObject *target);
    
};

