#include "transform.hpp"

void Transform::setLocalScale(const Vector3 &value) {
    m_scale = value;

    updateLocalMatrix();
}

Vector3 Transform::getLocalScale() const {
    return m_scale;
}

void Transform::setLocalRotation(const Quaternion &value) {
    m_rotation = value;

    updateLocalMatrix();
}

Quaternion Transform::getLocalRotation() const {
    return m_rotation;
}

Quaternion Transform::getRotation() const {
    return m_parent != nullptr ? (m_parent->m_rotation * m_rotation) : m_rotation;
}

void Transform::setLocalPosition(const Vector3 &value) {
    m_position = value;

    updateLocalMatrix();
}

void Transform::addLocalPosition(const Vector3 &value) {
    m_position += value;

    updateLocalMatrix();
}

Vector3 Transform::getLocalPosition() const {
    return m_position;
}

Vector3 Transform::getPosition() const {
    if (m_parent) {
        return Vector3(m_parent->getGlobalMatrix() * Vector4({m_position.x, m_position.y, m_position.z, 1.0f}));
    }
    return m_position;
}

Vector3 Transform::getRight() const {
    return getRotation() * Vector3::right;
}

Vector3 Transform::getUp() const {
    return getRotation() * Vector3::up;
}

Vector3 Transform::getForward() const {
    return getRotation() * Vector3::forward;
}

void Transform::setGlobalMatrix(const Matrix4x4 &matrix) {
    m_globalMatrix = matrix;
    
    for (auto & child : m_children) {
        child->updateLocalMatrix();
    }
}

Matrix4x4 Transform::getGlobalMatrix() const {
    return m_globalMatrix;
}

Matrix4x4 Transform::getLocalMatrix() const {
    return m_localMatrix;
}

bool Transform::hasParent() const {
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

Transform *Transform::getParent() const {
    return m_parent;
}

void Transform::addChildren(Transform *transform) {
    if (!transform || std::find(m_children.begin(), m_children.end(), transform) != m_children.end()) {
        return;
    }
    
    m_children.push_back(transform);
}

std::vector<Transform *> Transform::getChildren() const {
    return m_children;
}

bool Transform::hasChildren() const {
    return !m_children.empty();
}

void Transform::updateLocalMatrix() {
    m_localMatrix = Math::CreateTRS(m_scale, m_rotation, m_position);

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
