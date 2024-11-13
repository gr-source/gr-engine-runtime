#pragma once

#include "../Global.h"

#include "ecs/component.hpp"

class Transform : public Component {
public:
    Transform() = delete;

    void setLocalScale(const Vector3 &value);

    Vector3 getLocalScale() const;

    void setLocalRotation(const Quaternion &value);

    Quaternion getLocalRotation() const;

    Quaternion getRotation() const;

    void setLocalPosition(const Vector3 &value);

    void addLocalPosition(const Vector3 &value);

    Vector3 getPosition() const;

    Vector3 getLocalPosition() const;

    Vector3 getright() const;

    Vector3 getup() const;

    Vector3 getforward() const;

    void setGlobalMatrix(const Matrix4x4 &matrix);

    Matrix4x4 getGlobalMatrix() const;

    Matrix4x4 getLocalMatrix() const;

    bool hasParent() const;

    void setParent(Transform* transform, bool remove = true);

    Transform *getParent() const;

    void addChildren(Transform *transform);

    std::vector<Transform *> getChildren() const;

    bool hasChildren() const;

private:
    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;

    std::vector<Transform *> m_children;

    Transform* m_parent;

    Matrix4x4 m_localMatrix;

    Matrix4x4 m_globalMatrix;

    void updateLocalMatrix();

    void updateGlobalMatrix();
};





