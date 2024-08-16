#include "animator.hpp"

#include "transform.hpp"

#include "../renderer.h"
#include "../timer.h"

Animator::Animator() : m_transitioning(false), m_currentTransitionTime(0.0f), m_finalBoneMatrices(100, Matrix4x4::identityMatrix) {}

void Animator::UpdateAnim() {
    if (!m_armature) {
        return;
    }

    if (!m_animation && !m_animations.empty()) {
        m_animation = m_animations[0].get();
    }

    if (!m_animation) {
        return;
    }

    if (m_currentTime >= m_animation->getDuration()) {
        m_currentTime = 0.0f;
    }

    // m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    auto &parent = m_armature->getTransform();
    hierarchy(&parent, Matrix4x4::identityMatrix);

    m_currentTime += m_animation->getTick() * Time::deltaTime;

    if (m_transitioning) {
        m_currentTransitionTime += Time::deltaTime;
        if (m_currentTransitionTime >= 8.0f) {
            m_transitioning = false;
            m_currentTransitionTime = 8.0f;
        }
    }
}

void Animator::insert(std::unique_ptr<Animation> animation) {
    m_animations.push_back(std::move(animation));
}

void Animator::hierarchy(Transform *transform, const Matrix4x4 &parentTransform) {
    auto gameObject = transform->getGameObject();

    Matrix4x4 result = parentTransform;

    if (!gameObject) {
        return;
    }

    auto name = gameObject->getName();

    auto keyframe = m_animation->findKeyframe(name);
    auto bone = findBone(name);

    if (keyframe && bone) {
        Matrix4x4 offset = bone->getLocalMatrix();

        Matrix4x4 animMatrix = keyframe->getLocalMatrix(m_currentTime);

        if (m_transitioning && m_nextAnimation) {
            auto nextKeyframe = m_nextAnimation->findKeyframe(name);
            if (nextKeyframe) {
                Matrix4x4 nextAnimMatrix = nextKeyframe->getLocalMatrix(m_currentTime);

                float t = m_currentTransitionTime / 8.0f;
                animMatrix = animMatrix * (1.0f - t) + nextAnimMatrix * t;
            }
        }
        else if (m_nextAnimation) {
            // Transition has ended, switch to the next animation
            m_animation = m_nextAnimation;
            m_nextAnimation = nullptr;
            m_transitioning = false;
        }
        result *= animMatrix;

        m_finalBoneMatrices[bone->getBoneIndex()] = result * offset;
    }

    for (const auto &child : transform->getChildren()) {
        hierarchy(child, result);
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



