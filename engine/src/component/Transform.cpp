#include "Transform.h"

#include "../gameObject.h"

#include "../ui/View.h"

#include "../timer.h"

#include "cameraComponent.h"

Transform::Transform() : Component(true), position(Vector3::zero), scale(Vector3::one), rotation(Quaternion::identity), localPosition(Vector3::zero), parent(nullptr) {}

void Transform::OnInit() {
    OnUpdateParents();
}

void Transform::OnUpdate() {
    if (hasparent())
        OnUpdateParents();
}

void Transform::OnUpdateParents() {
    
    std::for_each(parents.begin(), parents.end(), [&](GameObject* parent) {
        Transform& transform = parent->gettransform();

        Matrix4x4 result = getGlobalMatrix() * transform.getLocalMatrix();

        result.decompose(transform.scale, transform.rotation, transform.position);
    });
    
}

Vector3 Transform::getright() const {
    return rotation * Vector3::right;
}

Vector3 Transform::getup() const {
    return rotation * Vector3::up;
}

Vector3 Transform::getforward() const {
    return rotation * Vector3::forward;
}

Matrix4x4 Transform::getGlobalMatrix() {
    return Math::translate(position) * Math::rotate(rotation) * Math::scale(scale);
}

Matrix4x4 Transform::getLocalMatrix() {
    return Math::translate(localPosition) * Math::rotate(localRotation) * Math::scale(localScale);
}

bool Transform::hasparent() const {
    return !parents.empty();
}

void Transform::addparent(GameObject* parent) {
    parents.push_back(parent);
}

std::vector<GameObject*>& Transform::getparents() {
    return parents;
}

void Transform::OnGui() {
    if (ImGui::TreeNode("Transform")) {
        const char* text[] {"X", "Y", "Z"};

        static int radioButtonIndex = 0;

        if (ImGui::RadioButton("Global", radioButtonIndex == 0)) {
            radioButtonIndex = 0;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Local", radioButtonIndex == 1)) {
            radioButtonIndex = 1;
        }

        ImGui::Text("Posição:");
        ImGui::DragFloatEx("##Posicao", text, (radioButtonIndex == 0 ? position.data : localPosition.data), 1.0f, 0.0f, 0.0f, "%.3f");

        ImGui::Text("Escala:");
        ImGui::DragFloatEx("##Escala", text, (radioButtonIndex == 0 ? scale.data : localScale.data), 1.0f, 0.0f, 0.0f, "%.3f");

        static float r[4]{0.0f, 0.0f, 0.0f};

        ImGui::Text("Rotação:");
        if (ImGui::DragFloatEx("##Rotacao", text, r, 1.0f, -360.0f, 360.0f)) {
            if (radioButtonIndex == 0) {
                rotation = Math::normalize(Quaternion::Euler({Math::rad(r[0]), Math::rad(r[1]), Math::rad(r[2])}));
                // rotation = Math::normalize(rotation);
            } else {
                localRotation = Math::normalize(Quaternion::Euler({Math::rad(r[0]), Math::rad(r[1]), Math::rad(r[2])}));
            }
        }
        ImGui::TreePop();
    }
}





