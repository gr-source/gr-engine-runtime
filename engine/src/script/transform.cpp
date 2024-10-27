#include "transform.hpp"

#include "../ecs/gameObject.h"

void Transform::setLocalScale(const Vector3 &value) {
    m_scale = value;

    updateLocalMatrix();
}

const Vector3 Transform::getLocalScale() const {
    return m_scale;
}

void Transform::setLocalRotation(const Quaternion &value) {
    m_rotation = value;

    updateLocalMatrix();
}

const Quaternion Transform::getLocalRotation() const {
    return m_rotation;
}

const Quaternion Transform::getRotation() const {
    if (m_parent)
        return (m_parent->m_rotation * m_rotation);
    return m_rotation;
}

void Transform::setLocalPosition(const Vector3 &value) {
    m_position = value;

    updateLocalMatrix();
}

void Transform::addLocalPosition(const Vector3 &value) {
    m_position += value;

    updateLocalMatrix();
}

const Vector3 &Transform::getLocalPosition() const {
    return m_position;
}

const Vector3 Transform::getPosition() const {
    if (m_parent) {
        return Vector3(m_parent->getGlobalMatrix() * Vector4({m_position.x, m_position.y, m_position.z, 1.0f}));
    }
    return m_position;
}

const Vector3 Transform::getright() const {
    return getRotation() * Vector3::right;
}

const Vector3 Transform::getup() const {
    return getRotation() * Vector3::up;
}

const Vector3 Transform::getforward() const {
    return getRotation() * Vector3::forward;
}

void Transform::setGlobalMatrix(const Matrix4x4 &matrix) {
    m_globalMatrix = matrix;
    
    for (auto & child : m_children) {
        child->updateLocalMatrix();
    }
}

const Matrix4x4 &Transform::getGlobalMatrix() const {
    return m_globalMatrix;
}

const Matrix4x4 &Transform::getLocalMatrix() const {
    return m_localMatrix;
}

const bool Transform::hasParent() const {
    return m_parent != nullptr;
}

void Transform::setParent(Transform *transform, bool remove) {
    if (remove && m_parent) {
        m_parent->m_children.erase(std::remove(m_parent->m_children.begin(), m_parent->m_children.end(), this), m_parent->m_children.end());
    }
    
    if (transform)
        transform->addChildren(this);
    m_parent = transform;

    updateLocalMatrix();
}

Transform *Transform::getParent() {
    return m_parent;
}

void Transform::addChildren(Transform *transform) {
    if (!transform || std::find(m_children.begin(), m_children.end(), transform) != m_children.end()) {
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

void Transform::updateLocalMatrix() {
    m_localMatrix = Math::CreateTRS(m_scale, m_rotation, m_position);//Math::translate<Matrix4x4>(m_position) * Math::rotate<Matrix4x4>(m_rotation) * Math::scale<Matrix4x4>(m_scale);

    updateGlobalMatrix();
}

void Transform::updateGlobalMatrix() {
    m_globalMatrix = getLocalMatrix();

    if (m_parent) {
        m_globalMatrix = m_parent->getGlobalMatrix() * m_globalMatrix;
    }

    for (auto & child : m_children) {
        child->updateLocalMatrix();
    }
}
