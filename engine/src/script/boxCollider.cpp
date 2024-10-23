#include "boxCollider.hpp"

#include "physics/physicsScene.hpp"

void BoxCollider::build(const Vector3 &size) {
    m_CollisionShape = std::make_shared<btBoxShape>(btVector3(1.0f, 1.0f, 1.0f));
    m_CollisionShape->setLocalScaling(btVector3(size.x, size.y, size.z));
}

btCollisionShape *BoxCollider::getShape() {
    return &(*m_CollisionShape);
}
