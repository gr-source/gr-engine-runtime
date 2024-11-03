#pragma once

#include "../ecs/gameObject.hpp"

class Scene {
public:
    static void Delete(GameObject *gameObject);

    static void insert(GameObject *gameObject);

    static void Remove(GameObject *gameObject);
};
