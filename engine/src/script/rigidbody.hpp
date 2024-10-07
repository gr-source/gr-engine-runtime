#pragma once

#include "../ecs/component.h"

#include "default.hpp"

class Rigidbody : public Component {
public:
    Rigidbody() = delete;

    void setup();

    void process();

    float getMass() const;

    void setMass(float value);

    bool getDinamic();

    void setDynamic(bool value);

private:
    std::shared_ptr<btDefaultMotionState> m_DefaultMotionState;

    std::shared_ptr<btRigidBody> m_body;

    btCollisionShape *m_collisionShape;

    Transform *m_transform;

    float m_mass;

    bool m_bDinamic;
};




