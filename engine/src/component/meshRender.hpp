#pragma once

#include "../ecs/entity.h"

class MeshRender : public Component {
public:
    static std::vector<GameObject*> &GetGameObjectFromRender();
};



