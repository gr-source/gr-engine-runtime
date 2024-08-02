#pragma once

#include "../ecs/entity.h"

class Animator : public Component {
public:
private:
    GameObject *m_armature;

    friend class Game;
};