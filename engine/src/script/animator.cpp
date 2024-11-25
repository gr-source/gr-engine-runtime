#include "animator.hpp"

#include "core/animation/animation.hpp"
#include "core/timer.hpp"

#include "script/transform.hpp"

#include "ecs/gameObject.hpp"

#include "renderer.h"

void Animator::update() {
    if (!isReady()) { return; }

    m_currentTime += m_animation->getTick() * TimeLast::deltaTime;
    m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    auto transform = m_armature->getTransform();

    for_each(transform);
}

bool Animator::isReady() const {
    return m_armature != nullptr && m_animation != nullptr;
}

GameObject *Animator::getArmature() const {
    return m_armature;
}

void Animator::setArmature(GameObject *armature) {
    m_armature = armature;
}

Animation *Animator::getAnimation() const {
    return m_animation;
}

void Animator::setAnimation(Animation *animation) {
    m_animation = animation;
}

Matrix4x4 *Animator::getBoneData() {
    return m_FinalBoneMatrices.data();
}

void Animator::for_each(Transform *transform, Matrix4x4 parent) {
    auto keyframe = m_animation->findKeyframe(transform->getGameObject()->getName());
    if (keyframe) {
        parent *= keyframe->getLocalMatrix(m_currentTime);

        m_FinalBoneMatrices[0] = parent * transform->getLocalMatrix();
    }

    for (auto child : transform->getChildren()) {
        for_each(child, parent);
    }
}


