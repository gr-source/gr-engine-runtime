#pragma once

#include "../ecs/entity.h"

class Terrain : public Component {
public:
    void OnInit() override;

private:
    void build();
};

