#include "animator.hpp"

#include "transform.hpp"

#include "../renderer.h"
#include "../timer.h"

Animator::Animator() : m_armature(nullptr), m_animation(nullptr), m_nextAnimation(nullptr),  m_currentTime(0.0f), m_finalBoneMatrices(100, Matrix4x4::identityMatrix) {}

void Animator::UpdateAnim() {
    if (!m_armature) {
        return;
    }

    if (!m_animation && !m_animations.empty()) {
        m_animation = m_animations[0].get();
    }

    if (!m_animation || !m_nextAnimation) {
        return;
    }

    // m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    // auto &parent = m_armature->getTransform();
    blendAnimation(m_animation, m_nextAnimation, 0.8f);
    // hierarchy(&parent, Matrix4x4::identityMatrix);

    // m_currentTime += m_animation->getTick() * Time::deltaTime;
}

void Animator::insert(std::unique_ptr<Animation> animation) {
    m_animations.push_back(std::move(animation));
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
    Animation* after,
    Animation* befor,
    const float currentTimeBase, const float currentTimeLayered,
    const Matrix4x4& parentTransform,
    const float blendFactor) {

        Matrix4x4 result = parentTransform;

        auto gameObject = transform->getGameObject();

        const std::string& name = gameObject->getName();

        auto bone = findBone(name);
        auto afterKeyframe = after->findKeyframe(name);
        auto beforKeyframe = befor->findKeyframe(name);

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

            // glm::mat4 globalTransformation = parentTransform * blendedMat;

            result *= blendedMat;


            // result *= keyframe->getLocalMatrix(m_currentTime);

            m_finalBoneMatrices[bone->getBoneIndex()] = result * offset;
        }

        for (const auto &child : transform->getChildren()) {
        CalculateBlendedBoneTransform(child, after, befor, currentTimeBase, currentTimeLayered, result, blendFactor);

            // hierarchy(child, result);
        }
}

void Animator::blendAnimation(Animation *after, Animation *befor, float blendFactor) {
    auto diff = after->getDuration() / befor->getDuration();
    auto animSpeedMultiplierUp = (1.0f - blendFactor) * 1.0f + diff * blendFactor;

    diff = befor->getDuration() / after->getDuration();
    auto animSpeedMultiplierDown = (1.0f - blendFactor) * diff + 1.0f * blendFactor;

    static float currentTimeBase = 0.0f;
    currentTimeBase += after->getTick() * Time::deltaTime * animSpeedMultiplierUp;
    currentTimeBase = fmod(currentTimeBase, after->getDuration());

    static float currentTimeLayered = 0.0f;
    currentTimeLayered += befor->getTick() * Time::deltaTime * animSpeedMultiplierDown;
    currentTimeLayered = fmod(currentTimeLayered, befor->getDuration());

    auto &parent = m_armature->getTransform();
    CalculateBlendedBoneTransform(&parent, after, befor, currentTimeBase, currentTimeLayered, Matrix4x4::identityMatrix, blendFactor);
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
    auto it = std::find_if(m_bones.begin(), m_bones.end(), [&name](const auto& bone) {
        return bone.getName() == name;
    });
    if (it != m_bones.end()) {
        return &(*it);
    }
    return nullptr;
}



