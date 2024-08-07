#pragma once

#include "../ecs/entity.h"
#include "../Global.h"

class Transform : public Component {
public:
    Vector3 scale;

    Quaternion rotation;

    Vector3 position;

    Transform();

    const Vector3 getright() const;

    const Vector3 getup() const;

    const Vector3 getforward() const;

    const Vector3 getPosition() const;

    const Matrix4x4 getGlobalMatrix() const;

    const Matrix4x4 getLocalMatrix() const;

    const bool hasParent() const;

    void setParent(Transform* transform);

    Transform *getParent();

    void addChildren(Transform *transform);

    std::vector<Transform *> &getChildren();

    const bool hasChildren() const;

private:
    std::vector<Transform *> m_children;

    Transform* m_parent;
};





