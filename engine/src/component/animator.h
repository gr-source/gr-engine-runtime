#pragma once

#include "../ecs/Entity.h"
/*
#include "../Global.h"

#include "../animation/animationClip.h"
#include "../animation/skeleton.h"
#include "../animation/bone.h"

#include "../filmbox/BoneInfo.h"

#include "Transform.h"
*/
class Animator : public Component {
public:
    /*Animator(const Skeleton& skeleton);
    
    void OnInit() override;

    void OnStart() override;

    void OnUpdate() override;
    
    void OnGui() override;

    std::array<Matrix4x4, 100> m_FinalBoneMatrices;

    std::vector<AnimationClip> clips;

    void playAnimation(int id);
    void stopAnimation();

protected:
    Transform* transform;

    Skeleton m_skeleton;
    
    float currentTime;

    int animationID;

    float animationSpeed;

    GameObject* m_armature;

    void CalculateBoneTransform(Bone& bone, const Matrix4x4& parentTransform);*/
};
