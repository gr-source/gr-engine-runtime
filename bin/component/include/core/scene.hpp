#pragma once

#include "../ecs/gameObject.hpp"

class Scene {
public:
    static void Delete(GameObject *gameObject);

    static void Add(GameObject *gameObject);

    static void Remove(GameObject *gameObject);
};
