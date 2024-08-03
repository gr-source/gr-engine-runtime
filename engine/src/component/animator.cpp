#include "animator.hpp"

#include "transform.hpp"

#include "../timer.h"

void Animator::UpdateAnim() {
    if (!m_armature) {
        return;
    }

    if (!m_animation && !m_animations.empty()) {
        m_animation = &m_animations[0];
    }

    if (!m_animation) {
        return;
    }

    m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());
    m_currentTime += m_animation->getTick() * Time::deltaTime;

    auto &parent = m_armature->getTransform();
    hiearchy(&parent, Matrix4x4::identityMatrix);
}

void Animator::hiearchy(Transform *transform, const Matrix4x4 &parentTransform) {
    auto gameObject = transform->getGameObject();

    Matrix4x4 result = parentTransform;

    if (!gameObject) {
        return;
    }
    
    auto name = gameObject->getName();

    auto keyframe = m_animation->findKeyframe(name);
    auto bone = findBone(name);

    if (keyframe && bone) {
        result *= keyframe->getLocalMatrix(m_currentTime);

        Matrix4x4 offset = bone->getLocalMatrix();
        m_finalBoneMatrices[bone->getBoneIndex()] = result * offset;
    }

    for (const auto &child : transform->getChildren()) {
        hiearchy(child, result);
    }
}

Bone *Animator::findBone(const std::string &name) {
    auto it = std::find_if(m_bones.begin(), m_bones.end(), [&name](const auto& bone) {
        return bone.getName() == name;
    });
    if (it != m_bones.end()) {
        return &(*it);
    }
    return nullptr;
}



