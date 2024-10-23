#pragma once

#include "../ecs/entity.hpp"

#include "script/default.hpp"

class BoxCollider : public Component {
    CLASS_FUNCTIONS

public:
    BoxCollider();

private:
    std::shared_ptr<btCollisionShape> m_CollisionShape;

    void buildShape(const Vector3 &size);
};

REGISTER_COMPONENT(BoxCollider)
