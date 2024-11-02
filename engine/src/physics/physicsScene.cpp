#include "physicsScene.hpp"

Vector3 PhysicsScene::gravity = {0.0f, -9.8f, 0.0f};

std::unique_ptr<btDefaultCollisionConfiguration> PhysicsScene::collisionConfiguration = nullptr;

std::unique_ptr<btSequentialImpulseConstraintSolver> PhysicsScene::solver = nullptr;

std::unique_ptr<btDiscreteDynamicsWorld> PhysicsScene::dynamicsWorld = nullptr;

std::unique_ptr<btBroadphaseInterface> PhysicsScene::broadphase = nullptr;

std::unique_ptr<btCollisionDispatcher> PhysicsScene::dispatcher = nullptr;

std::mutex PhysicsScene::m_mutex;

void PhysicsScene::Initialize() {
    broadphase             = std::make_unique<btDbvtBroadphase>();

    collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();

    dispatcher             = std::make_unique<btCollisionDispatcher>(&(*collisionConfiguration));

    solver                 = std::make_unique<btSequentialImpulseConstraintSolver>();

    dynamicsWorld          = std::make_unique<btDiscreteDynamicsWorld>(&(*dispatcher), &(*broadphase), &(*solver), &(*collisionConfiguration));

    dynamicsWorld->setGravity(VEC3_TO_BTVEC3(gravity));
}

void PhysicsScene::Shutdown() {
}

void PhysicsScene::AddRigidbody(btRigidBody* body) {
    dynamicsWorld->addRigidBody(body);
}

void PhysicsScene::RemoveRigidbody(btRigidBody* body) {
    dynamicsWorld->removeRigidBody(body);
}

void PhysicsScene::Update(float dt) {
    dynamicsWorld->stepSimulation(dt);
}

bool PhysicsScene::Raycast(const Vector3 &rayStart, const Vector3 &rayEnd) {
    btCollisionWorld::ClosestRayResultCallback rayCallback({rayStart.x, rayStart.y, rayStart.z}, {rayEnd.x, rayEnd.y, rayEnd.z});
    dynamicsWorld->rayTest({rayStart.x, rayStart.y, rayStart.z}, {rayEnd.x, rayEnd.y, rayEnd.z}, rayCallback);
    
    return rayCallback.hasHit();
}
