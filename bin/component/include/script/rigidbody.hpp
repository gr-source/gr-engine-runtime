#pragma once

#include "../ecs/entity.hpp"

#include "script/default.hpp"

class Rigidbody : public Component {
    CLASS_FUNCTIONS
public:
    Rigidbody();
    ~Rigidbody();

    void Init() override;

    void Update() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    Vector3 getLinearVelocity() const;

    void setLinearVelocity(const Vector3 &value);

    void applyCentralImpulse(const Vector3 &value);

private:
    std::shared_ptr<btDefaultMotionState> m_DefaultMotionState;

    std::shared_ptr<btRigidBody> m_body;

    btCollisionShape *m_collisionShape;

    Transform *m_transform;

    float m_mass;

    bool m_bDinamic;

    Vector3 m_lookRotation;

    void initialize();

    void update();

    void shutdown();
};

REGISTER_COMPONENT(Rigidbody)


