#include "animator.h"
/*
#include "MeshRender.h"
#include "Transform.h"

#include "../animation/animation.h"

#include "../filmbox/BoneInfo.h"

#include "../ObjectManager.h"
#include "../GameObject.h"
#include "../timer.h"

Animator::Animator(const Skeleton& skeleton) : Component(true), m_skeleton(skeleton), animationSpeed(1.0f), m_armature(nullptr) {}

void Animator::OnInit() {
    transform = &entity->getComponent<Transform>();

    currentTime = 0.0f;
    animationID = 0;
}

void Animator::OnStart() {
}

void Animator::OnUpdate() {
    if (clips.empty() || animationID == -1 || animationID > clips.size()) return;

    AnimationClip& clip = clips[animationID];
    
    currentTime += clip.getticks() * animationSpeed * Time::deltaTime;

    currentTime = std::fmod(currentTime, clip.getduration());

    CalculateBoneTransform(m_skeleton.getroot(), Matrix4x4::identityMatrix);
}

void Animator::CalculateBoneTransform(Bone& bone, const Matrix4x4& parentTransform) {
    Matrix4x4 result = parentTransform;

    if (bone.getname() != "none") {
        AnimationClip& clip = clips[animationID];

        Keyframe* keyframe = clip.findKeyframe(bone.getname());
        if (keyframe != nullptr) {
            result *= keyframe->getlocalmatrix(currentTime);

            Matrix4x4 offset = bone.getlocalmatrix();
            m_FinalBoneMatrices[bone.id] = result * offset;
        }
    } else {
        result = Matrix4x4::identityMatrix;
    }

    for (auto& child : bone.getchildren()) {
        CalculateBoneTransform(child, result);
    }
}

void Animator::playAnimation(int id) {
    animationID = (id >= clips.size() ? -1 : id);
    currentTime = 0.0f;
}

void Animator::stopAnimation() {
    animationID = -1;
    currentTime = 0.0f;
}

void boneRender(Bone& bone) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;

    if (!bone.hasparent()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool isOpen = ImGui::TreeNodeEx(bone.getname().c_str(), flags);
    if (isOpen) {
        for (auto& child : bone.getchildren()) {

            boneRender(child);
        }
        ImGui::TreePop();
    }
}

void Animator::OnGui() {
    std::string animationName("Pose\0");
    animationName += '\0';

    for (const auto& clip : clips) {
        animationName += clip.getname() + '\0';
    }
    animationName += '\0';

    if (ImGui::TreeNode("Animator")) {
        ImGui::Text("Animações");

        static int itemID = 0;
        if (ImGui::Combo("##Animations", &itemID, animationName.c_str())) {
            if (itemID == 0) {
                m_FinalBoneMatrices.fill(Matrix4x4::identityMatrix);
            }
            playAnimation(itemID - 1);
        }

        ImGui::Text("Velocidade de animação:");
        ImGui::DragFloat("#AnimationVelocity", &animationSpeed, 0.0f, 10.0f);

        if (ImGui::TreeNode("Skeleton")) {
            boneRender(m_skeleton.getroot());
            ImGui::TreePop();
        }

        ImGui::Text("Armadura:");
        if (m_armature != nullptr) {
            ImGui::Text("%s", m_armature->name.c_str());
        } else {
            ImGui::Text("Drop object armature");
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload("##GameObject", ImGuiDragDropFlags_None)) {
                std::string name(reinterpret_cast<char*>(data->Data), data->DataSize);
                if (!name.empty()) {
                    m_armature = ObjectManager::find(name);
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        ImGui::TreePop();
    }
}
*/

