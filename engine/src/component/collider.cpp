#include "collider.h"

Collider::Collider(ColliderType type) {
    switch (type) {
    case ColliderType::BoxCollider:
        m_CollisionShape = std::make_unique<btBoxShape>(btVector3(1.0f, 1.0f, 1.0f));
        break;
    case ColliderType::MeshCollider:
        // m_CollisionShape = std::make_unique<btBvhTriangleMeshShape>();
    default:
        break;
    }
}

btCollisionShape *Collider::getshape() {
    return m_CollisionShape.get();
}
