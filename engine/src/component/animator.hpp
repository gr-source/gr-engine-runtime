#pragma once

#include "../ecs/entity.h"

#include "../core/animation/animation.hpp"
#include "../core/animation/bone.hpp"

class Animator : public Component {
public:
    std::vector<Matrix4x4> m_finalBoneMatrices;

private:
    GameObject *m_armature;

    std::vector<Bone> m_bones;

    std::vector<Animation> m_animations;

    Animation *m_animation;

    float m_currentTime;

    friend class Game;
};