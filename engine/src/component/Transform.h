#pragma once

#include "../ecs/entity.h"
#include "../Global.h"

class Transform : public Component {
public:
    Vector3 scale;

    Quaternion rotation;

    Vector3 position;

    Transform* parent;

    std::vector<GameObject*> parents;

    Transform();

    const Vector3 getright() const;

    const Vector3 getup() const;

    const Vector3 getforward() const;

    const Vector3 getposition() const;

    const Matrix4x4 getglobalmatrix() const;

    const Matrix4x4 getLocalMatrix() const;

    bool hasparent() const;

    void addparent(GameObject* parent);

    std::vector<GameObject*>& getparents();

};




