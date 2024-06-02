#include "Transform.h"

#include "../gameObject.h"

#include "../ui/View.h"

#include "../timer.h"

#include "cameraComponent.h"

Transform::Transform() : Component(true), position(Vector3::zero), scale(Vector3::one), rotation(Quaternion::identity), parent(nullptr) {}

const Vector3 Transform::getright() const {
    if (parent)
        return Math::normalize((parent->rotation * rotation) * Vector3::right);
    return rotation * Vector3::right;
}

const Vector3 Transform::getup() const {
    if (parent)
        return Math::normalize((parent->rotation * rotation) * Vector3::up);
    return rotation * Vector3::up;
}

const Vector3 Transform::getforward() const {
    if (parent)
        return Math::normalize((parent->rotation * rotation) * Vector3::forward);
    return rotation * Vector3::forward;
}

const Vector3 Transform::getposition() const {
    if (parent) {
        return parent->getposition() - position;
    }
    return position;
}

const Matrix4x4 Transform::getglobalmatrix() const {
    Matrix4x4 localMatrix = Math::translate<Matrix4x4>(position) * Math::rotate<Matrix4x4>(rotation) * Math::scale<Matrix4x4>(scale);
    if (parent)
        return parent->getglobalmatrix() * localMatrix;
    return localMatrix;
}

const Matrix4x4 Transform::getLocalMatrix() const {
    return Math::translate<Matrix4x4>(position) * Math::rotate<Matrix4x4>(rotation) * Math::scale<Matrix4x4>(scale);
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

/*void Transform::OnGui() {
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (ImGui::TreeNode("Transform")) {
        const char* text[] {"X", "Y", "Z"};

        ImGui::Text("Posição:");
        ImGui::DragFloatEx("##Posicao", text, position.data, 1.0f, 0.0f, 0.0f, "%.3f");

        ImGui::Text("Escala:");
        ImGui::DragFloatEx("##Escala", text, scale.data, 1.0f, 1.0f, 0.0f, "%.3f");

        Vector3 euler = rotation.eulerAngles();

        ImGui::Text("Rotação:");
        if (ImGui::DragFloatEx("##Rotacao", text, euler.data, 1.0f, -360.0f, 360.0f)) {
            rotation = Math::normalize(Quaternion::Euler({Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)}));
        }
        ImGui::TreePop();
    }
}

*/



