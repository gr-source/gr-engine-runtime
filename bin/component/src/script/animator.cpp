#include "script/animator.hpp"

#include "core/function.hpp"

static void (*AnimatorUpdate)(Animator *);
INSERT_FUNCTION(0, AnimatorUpdate)

Animator::Animator() : m_armature(nullptr), m_animation(nullptr), m_currentTime(0.0f), m_FinalBoneMatrices(100, Matrix4x4::identityMatrix) {}

void Animator::Update() {
    return AnimatorUpdate(this);
    // std::cout << "Update" << std::endl;
}




