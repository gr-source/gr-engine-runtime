#include "Transform.h"

#include "../gameObject.h"

#include "../ui/View.h"

#include "../timer.h"

Transform::Transform() : position(Vector3::zero), scale(Vector3::one), rotation(Quaternion::identity), parent(nullptr), root(nullptr) {}

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

const Vector3 Transform::getPosition() const {
    if (parent) {
        return parent->getPosition() - position;
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

bool Transform::hasParent() const {
    return parent != nullptr;
}

void Transform::setParent(Transform *transform) {
    parent = transform;
}

Transform *Transform::getRoot() {
    return root;
}

void Transform::setRoot(Transform *transform) {
    root = transform;
}
