#include "animator.hpp"

#include "transform.hpp"

#include "../renderer.h"
#include "../timer.h"

void Animator::UpdateAnim() {
    if (!m_armature) {
        return;
    }

    auto &parent = m_armature->getTransform();

    /*
    if (m_animation != nullptr && m_nextAnimation != nullptr) {
        return blendAnimation(&parent, 0.5f);
    }
    */

    if (m_animation == nullptr && m_nextAnimation != nullptr) {
        m_animation = m_nextAnimation;
        m_nextAnimation = nullptr;
    }

    if (m_animation == nullptr) {
        return;
    }

    m_currentTime += m_animation->getTick() * TimeLast::deltaTime;

    if (m_currentTime >= m_animation->getDuration()) {
        // if (m_nextAnimation) {
        //     m_animation = m_nextAnimation;
        //     m_nextAnimation = nullptr;
        // } else {
            m_animation = nullptr;
        // }

        m_currentTime = 0;
    }

    if (m_animation)
        hierarchy(&parent, Matrix4x4::identityMatrix);
}

void Animator::insert(Animation *animation) {
    if (!animation) {
        return;
    }

    if (m_animations.find(animation->getName()) == m_animations.end()) {
        m_animations.emplace(animation->getName(), animation);
    }
}

Quaternion getRotationFromMatrix(const Matrix4x4 &m) {
    Vector3 s;
    Quaternion r;
    Vector3 t;

    m.decompose(s, r, t);
    return r;
}

void Animator::CalculateBlendedBoneTransform(
    Transform *transform,
    const float currentTimeBase, const float currentTimeLayered,
    const Matrix4x4& parentTransform,
    const float blendFactor) {

        Matrix4x4 result = parentTransform;

        auto gameObject = transform->getGameObject();

        const std::string& name = gameObject->getName();

        auto bone = findBone(name);
        auto afterKeyframe = m_animation->findKeyframe(name);
        auto beforKeyframe = m_nextAnimation->findKeyframe(name);

        if (bone && afterKeyframe && beforKeyframe) {
            Matrix4x4 offset = bone->getLocalMatrix();

            auto nodeTransform = afterKeyframe->getLocalMatrix(currentTimeBase);
            auto layeredNodeTransform = beforKeyframe->getLocalMatrix(currentTimeLayered);

             // Blend two matrices
            auto rot0 = getRotationFromMatrix(nodeTransform);
            auto rot1 = getRotationFromMatrix(layeredNodeTransform);

            auto finalRot = Math::slerp(rot0, rot1, blendFactor);
            
            auto blendedMat = Math::rotate<Matrix4x4>(finalRot);

            blendedMat.m30 = (1.0f - blendFactor) * nodeTransform.m30 + layeredNodeTransform.m30 * blendFactor;
            blendedMat.m31 = (1.0f - blendFactor) * nodeTransform.m31 + layeredNodeTransform.m31 * blendFactor;
            blendedMat.m32 = (1.0f - blendFactor) * nodeTransform.m32 + layeredNodeTransform.m32 * blendFactor;
            blendedMat.m33 = (1.0f - blendFactor) * nodeTransform.m33 + layeredNodeTransform.m33 * blendFactor;

            result *= blendedMat;

            m_finalBoneMatrices[bone->getBoneIndex()] = result * offset;
        }

        for (const auto &child : transform->getChildren()) {
            CalculateBlendedBoneTransform(child, currentTimeBase, currentTimeLayered, result, blendFactor);
        }
}

void Animator::blendAnimation(Transform *skeleton, float blendFactor) {
    auto diff = m_animation->getDuration() / m_nextAnimation->getDuration();
    auto animSpeedMultiplierUp = Math::lerp(1.0f, diff, blendFactor);

    diff = m_nextAnimation->getDuration() / m_animation->getDuration();
    auto animSpeedMultiplierDown = Math::lerp(1.0f, diff, blendFactor);

    m_currentTime += m_animation->getTick() * TimeLast::deltaTime * animSpeedMultiplierUp;
    m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    m_currentNextTime += m_nextAnimation->getTick() * TimeLast::deltaTime * animSpeedMultiplierDown;
    m_currentNextTime = std::fmod(m_currentNextTime, m_nextAnimation->getDuration());

    CalculateBlendedBoneTransform(skeleton, m_currentTime, m_currentNextTime, Matrix4x4::identityMatrix, blendFactor);
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

        result *= keyframe->getLocalMatrix(m_currentTime);

        m_finalBoneMatrices[bone->getBoneIndex()] = result * offset;
    }

    for (const auto &child : transform->getChildren()) {
        hierarchy(child, result);
    }
}

Bone *Animator::findBone(const std::string &name) {
    auto it = m_bones.find(name);
    if (it != m_bones.end()) {
        return &(*it).second;
    }
    return nullptr;
}



/*
    * Stop play animation of do not existis next animation:
    m_currentTime += m_animation->getTick() * Time::deltaTime;

    if (m_currentTime >= m_animation->getDuration()) {
        if (m_nextAnimation) {
            m_animation = m_nextAnimation;
            m_nextAnimation = nullptr;
        } else {
            m_animation = nullptr;
        }

        m_currentTime = 0;
    }

    if (m_animation)
        hierarchy(&parent, Matrix4x4::identityMatrix);
        
    * Playing next animation from waiting for end m_animation time

    m_currentTime += m_animation->getTick() * Time::deltaTime;

    if (m_currentTime >= m_animation->getDuration()) {
        if (m_nextAnimation) {
            m_animation = m_nextAnimation;
            m_nextAnimation = nullptr;
        }

        m_currentTime = 0;
    }

    if (m_animation)
        hierarchy(&parent, Matrix4x4::identityMatrix);

    * Playing of stop end animation

    if (m_animation == nullptr) {
        return;
    }

    m_currentTime += m_animation->getTick() * Time::deltaTime;

    if (m_currentTime >= m_animation->getDuration()) {
        m_animation = nullptr;
        m_currentTime = 0;
    }

    if (m_animation)
        hierarchy(&parent, Matrix4x4::identityMatrix);
}
*/