#pragma once

#include "core/animation/default.hpp"

#include "ecs/component.hpp"

class Animator : public Component {
public:
    Animator() = delete;

    void update();

    bool isReady() const;

private:
    GameObject *m_armature;

    Animation *m_animation;
};
