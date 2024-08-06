#include "transform.hpp"

#include "../ui/View.h"

#include "../timer.h"

Transform::Transform() : position(Vector3::zero), scale(Vector3::one), rotation(Quaternion::identity), m_parent(nullptr) {}

const Vector3 Transform::getright() const {
    if (m_parent)
        return Math::normalize((m_parent->rotation * rotation) * Vector3::right);
    return rotation * Vector3::right;
}

const Vector3 Transform::getup() const {
    if (m_parent)
        return Math::normalize((m_parent->rotation * rotation) * Vector3::up);
    return rotation * Vector3::up;
}

const Vector3 Transform::getforward() const {
    if (m_parent)
        return Math::normalize((m_parent->rotation * rotation) * Vector3::forward);
    return rotation * Vector3::forward;
}

const Vector3 Transform::getPosition() const {
    if (m_parent) {
        return Vector3(m_parent->getglobalmatrix() * Vector4(position, 1.0f));
    }
    return position;
}

const Matrix4x4 Transform::getglobalmatrix() const {
    Matrix4x4 localMatrix = Math::translate<Matrix4x4>(position) * Math::rotate<Matrix4x4>(rotation) * Math::scale<Matrix4x4>(scale);
    if (m_parent)
        return m_parent->getglobalmatrix() * localMatrix;
    return localMatrix;
}

const Matrix4x4 Transform::getLocalMatrix() const {
    return Math::translate<Matrix4x4>(position) * Math::rotate<Matrix4x4>(rotation) * Math::scale<Matrix4x4>(scale);
}

const bool Transform::hasParent() const {
    return m_parent != nullptr;
}

void Transform::setParent(Transform *transform) {
    if (m_parent) {
        m_parent->m_children.erase(std::remove(m_parent->m_children.begin(), m_parent->m_children.end(), this), m_parent->m_children.end());
    }
    
    if (transform)
        transform->addChildren(this);
    m_parent = transform;
}

void Transform::addChildren(Transform *transform) {
    auto it = std::find(m_children.begin(), m_children.end(), transform);
    if (it != m_children.end()) {
        return;
    }
    m_children.push_back(transform);
}

std::vector<Transform *> &Transform::getChildren() {
    return m_children;
}

const bool Transform::hasChildren() const {
    return !m_children.empty();
}






