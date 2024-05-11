#pragma once

#include "../Global.h"
#include "../ecs/Entity.h"

#include "../physics/physicsScene.h"

struct RigidbodyDef {
    Vector3 loockRotation;

    float mass;

    bool isDynamic;

    float friction;

    float restitution;

    RigidbodyDef() : loockRotation(Vector3::zero), mass(1.0f), friction(1.0f), restitution(0.3f), isDynamic(false) {}
};

class Rigidbody : public Component {
public:
    Rigidbody(const RigidbodyDef& def = RigidbodyDef());
    ~Rigidbody();

    void OnStart() override;

    void OnInit() override;

    void OnUpdate() override;

    void addimpulse(const Vector3& impulse);

    void addcentralforce(const Vector3& force);

    void addlinearvelocity(const Vector3& velocity);

    void OnGui() override;

    void GetAABB(Vector3& min, Vector3& max);
protected:    
    PhysicsScene* pPhysicsScene;

    Transform* m_transform;

    std::mutex m_mutex;

    Collider* m_collider;

    std::unique_ptr<btDefaultMotionState> pDefaultMotionState;
    
    std::unique_ptr<btRigidBody> body;

    RigidbodyDef def;
};



