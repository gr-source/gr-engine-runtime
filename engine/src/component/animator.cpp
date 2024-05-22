#include "animator.h"

#include "Transform.h"
/*#include "MeshRender.h"

#include "../animation/animation.h"

#include "../filmbox/BoneInfo.h"

#include "../ObjectManager.h"
#include "../GameObject.h"*/
#include "../timer.h"

Animator::Animator(const Skeleton& skeleton) : m_skeleton(skeleton) {}

void Animator::OnInit() {
    transform = &entity->getComponent<Transform>();

    currentTime = 0.0f;

    m_FinalBoneMatrices.fill(Matrix4x4::identityMatrix);
}

void Animator::OnStart() {
}

void Animator::OnUpdate() {
    if (m_animations.empty()) return;

    AnimationClip& animation = m_animations[0];
    
    currentTime += animation.getticks() * Time::deltaTime;

    currentTime = std::fmod(currentTime, animation.getduration());

    CalculateBoneTransform(m_skeleton.getroot(), Matrix4x4::identityMatrix);
}

void Animator::CalculateBoneTransform(const Bone& bone, const Matrix4x4& parentTransform) {
    Matrix4x4 result = parentTransform;

    AnimationClip& animation = m_animations[0];

    Keyframe* keyframe = animation.findKeyframe(bone.getname());
    if (keyframe != nullptr) {
        result *= keyframe->getlocalmatrix(currentTime);

        Matrix4x4 offset = bone.getlocalmatrix();
        m_FinalBoneMatrices[bone.id] = result * offset;
    }

    for (const auto& child : bone.getchildren()) {
        CalculateBoneTransform(child, result);
    }
}

