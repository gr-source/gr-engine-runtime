#pragma once

#include "../Global.h"
#include "../ecs/Component.h"

#include <bullet/btBulletDynamicsCommon.h>

enum struct ColliderType {
    NONE = 0,
    BoxCollider = 1,
    MeshCollider = 2
};

class Collider : public Component {
public:
    Collider(ColliderType type = ColliderType::NONE);

    std::unique_ptr<btCollisionShape> m_CollisionShape;

    btCollisionShape* getshape();
protected:
};