#include "animator.hpp"

#include "transform.hpp"

#include "../ecs/gameObject.hpp"

#include "core/time.hpp"

Animator::Animator() : m_armature(nullptr), m_animation(nullptr), m_nextAnimation(nullptr), m_currentTime(0.0f), m_currentNextTime(0.0f), m_finalBoneMatrices(100, Matrix4x4::identityMatrix), m_waitForEnd(false) {}

void Animator::play(const std::string &name) {
    auto it = std::find_if(m_animations.begin(), m_animations.end(), [&name](auto &animation) {
        return animation->getName() == name;
    });
    if (it != m_animations.end()) {
        m_nextAnimation = &(*(*it));
        // m_waitForEnd = true;
    }
}

void Animator::insert(Animation *animation) {
    m_animations.push_back(animation);
}

void Animator::setArmature(GameObject *armature) {
    m_armature = armature;
}
