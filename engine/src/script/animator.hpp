#pragma once

#include "../ecs/gameObject.h"

#include "../core/animation/animation.hpp"
#include "../core/animation/bone.hpp"

#include "default.hpp"

class Animator : public Component {
public:
    Animator();
    
    void UpdateAnim();

    void insert(Animation *animation);

    std::vector<Matrix4x4> m_finalBoneMatrices;

    template <typename ...Args>
    Bone &addBone(Args &&...args);

private:
    GameObject *m_armature;

    std::unordered_map<std::string, Bone> m_bones;

    std::unordered_map<std::string, Animation *> m_animations;

    Animation *m_animation;

    Animation *m_nextAnimation;

    float m_currentTime;
    
    float m_currentNextTime;

    bool m_waitForEnd;

    void blendAnimation(Transform *skeleton, float blendFactor);

    void CalculateBlendedBoneTransform(
        Transform *transform,
    const float currentTimeBase, const float currentTimeLayered,
    const Matrix4x4& parentTransform,
    const float blendFactor);

    void hierarchy(Transform *transform, const Matrix4x4 &parentTransform);

    Bone *findBone(const std::string &name);

    friend class Game;
};

template <typename ...Args>
inline Bone &Animator::addBone(Args &&...args) {
    auto &&[it, added] = m_bones.emplace(std::forward<Args>(args)...);

    return (*it).second;
}


