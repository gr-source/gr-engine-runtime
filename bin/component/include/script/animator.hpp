#pragma once

#include "ecs/entity.hpp"

#include "default.hpp"

#include "core/animation/animation.hpp"
#include "core/animation/bone.hpp"

class Animator : public Component {
    CLASS_FUNCTIONS
public:
    Animator();

    void Update() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;
    
    std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;
    
private:
    std::vector<Matrix4x4> m_FinalBoneMatrices;

    GameObject *m_armature;

    Animation *m_animation;

    float m_currentTime;
};

REGISTER_COMPONENT(Animator)




