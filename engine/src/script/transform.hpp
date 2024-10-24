#pragma once

#include "../ecs/entity.h"
#include "../Global.h"

class Transform : public Component {
public:
    Transform() = delete;

    void setLocalScale(const Vector3 &value);

    const Vector3 getLocalScale() const;

    void setLocalRotation(const Quaternion &value);

    const Quaternion getLocalRotation() const;

    const Quaternion getRotation() const;

    void setLocalPosition(const Vector3 &value);

    void addLocalPosition(const Vector3 &value);

    const Vector3 getPosition() const;

    const Vector3 &getLocalPosition() const;

    const Vector3 getright() const;

    const Vector3 getup() const;

    const Vector3 getforward() const;

    void setGlobalMatrix(const Matrix4x4 &matrix);

    const Matrix4x4 &getGlobalMatrix() const;

    const Matrix4x4 &getLocalMatrix() const;

    const bool hasParent() const;

    void setParent(Transform* transform, bool remove = true);

    Transform *getParent();

    void addChildren(Transform *transform);

    std::vector<Transform *> &getChildren();

    const bool hasChildren() const;

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





