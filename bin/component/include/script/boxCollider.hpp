#pragma once

#include "../ecs/component.hpp"

#include "script/default.hpp"

class BoxCollider : public Component {
public:
    BoxCollider();
    
private:
    std::shared_ptr<btCollisionShape> m_CollisionShape;
};

REGISTER_COMPONENT(BoxCollider)
