#include "meshCollider.h"

#include "Transform.h"

MeshCollider::MeshCollider(const Mesh &mesh) : m_transform(nullptr), m_TriangleMeshShape(nullptr), m_triangleMesh(std::make_unique<btTriangleMesh>()) {
    for (u32 i=0;i<mesh.getelementcount();i+=3) {
        const Vertex3D& vertex1 = mesh.getvertices()[mesh.getindices()[i]];
        const Vertex3D& vertex2 = mesh.getvertices()[mesh.getindices()[i + 1]];
        const Vertex3D& vertex3 = mesh.getvertices()[mesh.getindices()[i + 2]];

        Vector3 trangle1 = vertex1.position;
        Vector3 trangle2 = vertex2.position;
        Vector3 trangle3 = vertex3.position;

        m_triangleMesh->addTriangle(
            btVector3(trangle1.x, trangle1.y, trangle1.z),
            btVector3(trangle2.x, trangle2.y, trangle2.z),
            btVector3(trangle3.x, trangle3.y, trangle3.z)
        );
    }
}

void MeshCollider::OnInit() {
    m_transform = &entity->getComponent<Transform>();
    
    Collider& m_collider = entity->addComponent<Collider>(ColliderType::MeshCollider);
    
    m_collider.m_CollisionShape = std::make_unique<btBvhTriangleMeshShape>(m_triangleMesh.get(), true);

    m_TriangleMeshShape = reinterpret_cast<btTriangleMeshShape*>(m_collider.getshape());

    m_TriangleMeshShape->setLocalScaling(btVector3(m_transform->scale.x, m_transform->scale.y, m_transform->scale.z));
}

void MeshCollider::OnUpdate() {
}

void MeshCollider::OnGui() {
    if (ImGui::TreeNode("MeshCollider")) {
        ImGui::TreePop();
    }
}
