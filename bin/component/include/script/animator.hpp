#pragma once

#include "ecs/entity.hpp"

#include "default.hpp"

#include "core/animation/animation.hpp"
#include "core/animation/bone.hpp"

class Animator : public Component {
    CLASS_FUNCTIONS
public:
    Animator();
};

REGISTER_COMPONENT(Animator)




