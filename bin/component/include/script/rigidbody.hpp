#pragma once

#include "../ecs/entity.hpp"

#include "script/default.hpp"

class Rigidbody : public Component {
    CLASS_FUNCTIONS
public:
    enum CollisionFlags {
		CollisionFlags_STATIC_OBJECT = 1,
		CollisionFlags_KINEMATIC_OBJECT = 2,
		CollisionFlags_NO_CONTACT_RESPONSE = 4,
		CollisionFlags_CUSTOM_MATERIAL_CALLBACK = 8,  //this allows per-triangle material (friction/restitution)
		CollisionFlags_CHARACTER_OBJECT = 16,
		CollisionFlags_DISABLE_VISUALIZE_OBJECT = 32,          //disable debug drawing
		CollisionFlags_DISABLE_SPU_COLLISION_PROCESSING = 64,  //disable parallel/SPU processing
		CollisionFlags_HAS_CONTACT_STIFFNESS_DAMPING = 128,
		CollisionFlags_HAS_CUSTOM_DEBUG_RENDERING_COLOR = 256,
		CollisionFlags_HAS_FRICTION_ANCHOR = 512,
		CollisionFlags_HAS_COLLISION_SOUND_TRIGGER = 1024
	};

    Rigidbody();

    void Init() override;

    void Destroy() override;

    void Update() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    Vector3 getLinearVelocity() const;

    void setLinearVelocity(const Vector3 &value);

    void applyCentralForce(const Vector3 &value);

    void applyCentralImpulse(const Vector3 &value);

    void applyCentralPushImpulse(const Vector3 &value);

    void applyForce(const Vector3 &force, const Vector3 &pos);

    void applyImpulse(const Vector3 &impulse, const Vector3 &pos);

    void applyPushImpulse(const Vector3 &impulse, const Vector3 &pos);

    void applyTorque(const Vector3 &value);

    void applyTorqueImpulse(const Vector3 &value);

    void applyTorqueTurnImpulse(const Vector3 &value);

    float getMass() const;

    void setMass(float value);

    btRigidBody *getBody() const;

    float getMotionThreshold() const;

    void setMotionThreshold(float value);

    float getSweptSphereRadius() const;

    void setSweptSphereRadius(float value);

    void setPosition(const Vector3 &position);

    Vector3 getPosition() const;

    void setRotation(Quaternion q);

    Quaternion getRotation() const;

    void setFreezerRotation(const Vector3 &value);

    Vector3 getFreezerRotation() const;

    void setColissionFlags(int flags);

    int getColissionFlags() const;

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

REGISTER_COMPONENT(Rigidbody)


