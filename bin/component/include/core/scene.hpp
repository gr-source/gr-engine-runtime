#pragma once

#include <ecs/default.hpp>

class Scene {
public:
    static void Delete(GameObject *gameObject);

    static void Add(GameObject *gameObject);

    static void Remove(GameObject *gameObject);
};
