#pragma once

#include "../gameObject.h"

#include <vector>

class Scene {
public:
    static std::vector<GameObject *> &(*GetGameObject)();
};

