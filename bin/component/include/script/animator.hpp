#pragma once

#include "../ecs/entity.hpp"

#include "default.hpp"

#include "core/animation/animation.hpp"
#include "core/animation/bone.hpp"

class Animator : public Component {
    CLASS_FUNCTIONS

public:
    Animator();

    void play(const std::string &name);

    template <typename ...Args>
    Bone &addBone(Args &&...args);

    template <typename ...Args>
    Animation &addAnimation(Args &&...args);

    void insert(Animation *animation);

    void setArmature(GameObject *armature);

    std::vector<Matrix4x4> m_finalBoneMatrices;

private:
    GameObject *m_armature;

    std::unordered_map<std::string, Bone> m_bones;

    std::vector<Animation *> m_animations;

    Animation *m_animation;

    Animation *m_nextAnimation;

    float m_currentTime;
    
    float m_currentNextTime;

    bool m_waitForEnd;
};

template <typename... Args>
inline Bone &Animator::addBone(Args &&...args) {
    auto &&[it, added] = m_bones.emplace(std::forward<Args>(args)...);
    return (*it).second;
}

template <typename... Args>
inline Animation &Animator::addAnimation(Args &&...args) {
    return *m_animations.emplace_back(std::make_unique<Animation>(std::forward<Args>(args)...));
}


// REGISTER_COMPONENT(Animator)




