#pragma once

#include "default.hpp"

#include <ecs/component.hpp>

class Rigidbody : public Component {
public:
    Rigidbody() = delete;

    void initialize();

    void shutdown();

    void update();

    float getMass() const;

    void setMass(float value);

    Vector3 getFreezerRotation() const;

    void setFreezerRotation(const Vector3 &value);

    float getMotionThreshold() const;

    void setMotionThreshold(float value);

    float getSweptSphereRadius() const;

    void setSweptSphereRadius(float value);

private:
    std::shared_ptr<btDefaultMotionState> m_DefaultMotionState;

    std::shared_ptr<btRigidBody> m_body;

    btCollisionShape *m_collisionShape;

    Transform *m_transform;

    float m_mass;

    Vector3 m_lookRotation;

    float fMotionThreshold;

    float fSweptSphereRadius;
};




