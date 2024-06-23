#pragma once

#include <memory>
#include <mutex>

#include <bullet/btBulletDynamicsCommon.h>

#include "../common.h"

#define FROMVEC3(vec) btVector3(vec.x, vec.y, vec.z)
#define FROMQUAT(q) btQuaternion(q.x, q.y, q.z, q.w)

class PhysicsScene {
public:
    
    static PhysicsScene& getInstance();

    PhysicsScene();

    void addRigidbody(btRigidBody* body);

    void removeRigidbody(btRigidBody* body);

    void Simulate(float dt);

    static Vector3 gravity;

protected:
    std::unique_ptr<btBroadphaseInterface> broadphase;

    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;

    std::unique_ptr<btCollisionDispatcher> dispatcher;

    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

    std::mutex m_mutex;
};

