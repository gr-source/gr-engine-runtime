#pragma once

#include "collider.h"

#include "../ecs/Entity.h"
#include "../Global.h"

class BoxCollider : public Component {
public:
    Vector3 size;

    BoxCollider(const Vector3& size = Vector3::one);

    void OnInit() override;

protected:
    btBoxShape* m_boxShape;
};


