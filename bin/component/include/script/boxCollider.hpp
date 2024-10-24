#pragma once

#include "../ecs/entity.hpp"

#include "script/default.hpp"

class BoxCollider : public Component {
    CLASS_FUNCTIONS

public:
    BoxCollider();


    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

private:
    std::shared_ptr<btCollisionShape> m_CollisionShape;

    Vector3 boxSize;

    void buildShape(const Vector3 &size);
};

REGISTER_COMPONENT(BoxCollider)
