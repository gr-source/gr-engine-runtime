#pragma once

#include "ecs/component.h"

#include "default.hpp"

class BoxCollider : public Component {
public:
    BoxCollider() = delete;

    void build(const Vector3 &size);

    btCollisionShape *getShape();
    
private:
    std::shared_ptr<btCollisionShape> m_CollisionShape;
};

