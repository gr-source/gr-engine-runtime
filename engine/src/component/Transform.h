#pragma once

#include "../ecs/Entity.h"
#include "../Global.h"

class Transform : public Component {
public:
    Vector3 scale;
    Vector3 localScale;

    Quaternion rotation;
    Quaternion localRotation;

    Vector3 position;
    Vector3 localPosition;

    GameObject* parent;

    std::vector<GameObject*> parents;

    Transform();

    void OnInit() override;

    void OnUpdate() override;

    void OnUpdateParents();

    Vector3 getright() const;

    Vector3 getup() const;

    Vector3 getforward() const;

    Matrix4x4 getGlobalMatrix();

    Matrix4x4 getLocalMatrix();

    bool hasparent() const;

    void addparent(GameObject* parent);

    std::vector<GameObject*>& getparents();

    void OnGui() override;
};




