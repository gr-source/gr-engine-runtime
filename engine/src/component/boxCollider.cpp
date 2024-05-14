#include "boxCollider.h"

BoxCollider::BoxCollider(const Vector3& size) : size(size), m_boxShape(nullptr) {}

void BoxCollider::OnInit() {
    Collider& m_Collider = entity->addComponent<Collider>(ColliderType::BoxCollider);

    m_boxShape = reinterpret_cast<btBoxShape*>(m_Collider.getshape());
    m_boxShape->setLocalScaling(btVector3(size.x, size.y, size.z));
}

/*
void BoxCollider::OnGui() {
    if (ImGui::TreeNode("BoxCollider")) {
        ImGui::Text("Size:");
        if (ImGui::DragFloat3("##Size", size.data, 1.0f)) {
            m_boxShape->setLocalScaling(btVector3(size.x, size.y, size.z));
        }

        ImGui::TreePop();
    }
}
*/
