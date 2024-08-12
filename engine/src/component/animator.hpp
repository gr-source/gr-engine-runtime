#pragma once

#include "../ecs/gameObject.h"

#include "../core/animation/animation.hpp"
#include "../core/animation/bone.hpp"

#include "default.hpp"

class Animator : public Component {
public:
    Animator();
    
    void UpdateAnim();

    std::vector<Matrix4x4> m_finalBoneMatrices;

private:
    GameObject *m_armature;

    std::vector<Bone> m_bones;

    std::vector<std::unique_ptr<Animation>> m_animations;

    Animation *m_animation;

    float m_currentTime;

    void hierarchy(Transform *transform, const Matrix4x4 &parentTransform);

    Bone *findBone(const std::string &name);

    friend class Game;
};