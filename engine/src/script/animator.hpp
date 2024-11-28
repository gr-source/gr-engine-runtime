#pragma once

#include "core/animation/default.hpp"

#include <ecs/component.hpp>

#include "script/default.hpp"

class Animator : public Component {
public:
    Animator() = delete;

    void update();

    bool isReady() const;

    GameObject *getArmature() const;

    void setArmature(GameObject *armature);

    Animation *getAnimation() const;

    void setAnimation(Animation *animation);

    Matrix4x4 *getBoneData();

private:
    std::vector<Matrix4x4> m_FinalBoneMatrices;

    GameObject *m_armature;

    Animation *m_animation;

    float m_currentTime;

    void for_each(Transform *transform, Matrix4x4 parent = Matrix4x4::identityMatrix);
};
