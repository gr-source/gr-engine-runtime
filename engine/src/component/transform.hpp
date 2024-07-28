#pragma once

#include "../ecs/entity.h"
#include "../Global.h"

class Transform : public Component {
public:
    Vector3 scale;

    Quaternion rotation;

    Vector3 position;

    Transform* parent;

    Transform* root;

    Transform();

    const Vector3 getright() const;

    const Vector3 getup() const;

    const Vector3 getforward() const;

    const Vector3 getPosition() const;

    const Matrix4x4 getglobalmatrix() const;

    const Matrix4x4 getLocalMatrix() const;

    bool hasParent() const;

    void setParent(Transform* transform);

    Transform* getRoot();

    void setRoot(Transform* transform);
};





