#include "physicsScene.h"

Vector3 PhysicsScene::gravity = {0.0f, -9.8f, 0.0f};

PhysicsScene& PhysicsScene::getInstance() {
    static PhysicsScene pPhysicsScene;
    return pPhysicsScene;
}

PhysicsScene::PhysicsScene() : broadphase(nullptr), collisionConfiguration(nullptr), dispatcher(nullptr), solver(nullptr), dynamicsWorld(nullptr) {
    broadphase = std::make_unique<btDbvtBroadphase>();

    collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();

    dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());

    solver = std::make_unique<btSequentialImpulseConstraintSolver>();

    dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfiguration.get());

    dynamicsWorld->setGravity(FROMVEC3(gravity));
}

void PhysicsScene::addRigidbody(btRigidBody* body) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dynamicsWorld->addRigidBody(body);
}

void PhysicsScene::removeRigidbody(btRigidBody* body) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dynamicsWorld->removeRigidBody(body);
}

void PhysicsScene::Simulate(float dt) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dynamicsWorld->stepSimulation(dt);
}

