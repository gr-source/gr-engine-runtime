#pragma once

#include "../ecs/entity.hpp"

#include "script/default.hpp"

class Rigidbody : public Component {
    CLASS_FUNCTIONS
public:
    Rigidbody();

    void Init() override;

    void Update() override;


private:
    std::shared_ptr<btDefaultMotionState> m_DefaultMotionState;

    std::shared_ptr<btRigidBody> m_body;

    btCollisionShape *m_collisionShape;

    Transform *m_transform;

    float m_mass;

    bool m_bDinamic;

    void setup();

    void process();
};

REGISTER_COMPONENT(Rigidbody)


