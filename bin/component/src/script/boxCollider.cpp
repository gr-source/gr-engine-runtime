#include "script/boxCollider.hpp"

#include "script/transform.hpp"

BoxCollider::BoxCollider() : m_CollisionShape(nullptr) {
    buildShape(Vector3::one);
}

void BoxCollider::buildShape(const Vector3 &size) {
}








