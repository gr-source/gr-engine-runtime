#pragma once

#include "../ecs/entity.h"

#include "../animation/animationClip.h"
#include "../animation/skeleton.h"

class Animator : public Component {
public:
    Animator(const Skeleton& skeleton);
    
    void OnInit() override;

    void OnStart() override;

    void OnUpdate() override;
    
    std::array<Matrix4x4, 100> m_FinalBoneMatrices;
    
    std::vector<AnimationClip> m_animations;

protected:
    Transform* transform;

    Skeleton m_skeleton;
    
    float currentTime;

    void CalculateBoneTransform(const Bone* bone, const Matrix4x4& parentTransform);

    bool loaded;

    friend class Game;
};
