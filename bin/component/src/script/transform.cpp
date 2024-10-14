#include "script/transform.hpp"

#include "ecs/gameObject.hpp"

#include <algorithm>

Transform::Transform() : m_scale(Vector3::one), m_rotation(Quaternion::identity), m_position(Vector3::zero), m_parent(nullptr), m_localMatrix(Matrix4x4::identityMatrix), m_globalMatrix(Matrix4x4::identityMatrix) {}

void Transform::setLocalScale(const Vector3 &value) {
    m_scale = value;

    updateLocalMatrix();
}

void Transform::setLocalRotation(const Quaternion &value) {
    m_rotation = value;

    updateLocalMatrix();
}

const Quaternion Transform::getLocalRotation() const {
    return m_rotation;
}

const Quaternion Transform::getRotation() const {
    if (m_parent) {
        return m_parent->m_rotation * m_rotation;
    }
    return m_rotation;
}

void Transform::setLocalPosition(const Vector3 &value) {
    m_position = value;

    updateLocalMatrix();
}

const Vector3 Transform::getLocalPosition() const {
    return m_position;
}

void Transform::addLocalPosition(const Vector3 &value) {
    m_position += value;

    updateLocalMatrix();
}

const Vector3 Transform::getPosition() const {
    if (m_parent) {
        return Vector3(m_parent->getGlobalMatrix() * Vector4(m_position, 1.0f));
    }
    return m_position;
}

const Vector3 Transform::getLocalForward() const {
    return m_rotation * Vector3::forward;
}

const Vector3 Transform::getForward() const {
    return getRotation() * Vector3::forward;
}

const Vector3 Transform::getRight() const {
    return getRotation() * Vector3::right;
}

const Vector3 Transform::getUp() const {
    return getRotation() * Vector3::up;
}

const Matrix4x4 &Transform::getGlobalMatrix() const {
    return m_globalMatrix;
}

const Matrix4x4 &Transform::getLocalMatrix() const {
    return m_localMatrix;
}

void Transform::setParent(Transform *transform) {
    if (m_parent) {
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

void Transform::addChildren(Transform *child) {
    if (!child || std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
        return;
    }
    
    m_children.push_back(child);
}

Transform *Transform::getChildren(const std::string &name) {
    auto it = std::find_if(m_children.begin(), m_children.end(), [name] (Transform *transform) {
        return transform->getGameObject()->getName() == name;
    });
    if (it == m_children.end()) {
        return nullptr;
    }
    return (*it);
}

std::vector<Transform *> &Transform::getChildren() {
    return m_children;
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

std::ostream &Transform::Serialize(std::ostream &os) {
    os.write(reinterpret_cast<const char *>(&m_scale), sizeof(Vector3));
    os.write(reinterpret_cast<const char *>(&m_rotation), sizeof(Quaternion));
    os.write(reinterpret_cast<const char *>(&m_position), sizeof(Vector3));
    return os;
}

std::istream &Transform::Deserialize(std::istream &is) {
    is.read(reinterpret_cast<char *>(&m_scale), sizeof(Vector3));
    is.read(reinterpret_cast<char *>(&m_rotation), sizeof(Quaternion));
    is.read(reinterpret_cast<char *>(&m_position), sizeof(Vector3));
    updateLocalMatrix();
    return is;
}
