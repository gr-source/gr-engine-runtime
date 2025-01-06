#include "rigidbody.hpp"

#include "meshCollider.hpp"

#include "physics/physicsScene.hpp"

#include "script/boxCollider.hpp"
#include "script/transform.hpp"

#include "core/console.hpp"

#include "ecs/gameObject.hpp"
#include "ecs/entity.hpp"

void Rigidbody::initialize() {
    if (entity->hasComponent<MeshCollider>()) {
        m_collisionShape = entity->getComponent<MeshCollider>()->getShape();
    } else if (entity->hasComponent<BoxCollider>()) {
        m_collisionShape = entity->getComponent<BoxCollider>()->getShape();
    }

    if (!m_collisionShape) {
        return Console::LogError("[ Rigidbody ] Shape not found.");
    }

    m_DefaultMotionState = std::make_shared<btDefaultMotionState>(
		btTransform(
			QUAT_TO_BTQUAT(m_transform->getRotation()),
			VEC3_TO_BTVEC3(m_transform->getPosition())
		)
	);

    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
    m_collisionShape->calculateLocalInertia(m_mass, fallInertia);

    btRigidBody::btRigidBodyConstructionInfo pRigidBodyConstructionInfo(m_mass, &(*m_DefaultMotionState), m_collisionShape, fallInertia);
    pRigidBodyConstructionInfo.m_friction = 0.5f;
    pRigidBodyConstructionInfo.m_restitution = 0.1f;

    if (m_body) {
        shutdown();
        m_body = nullptr;
    }

    m_body = std::make_shared<btRigidBody>(pRigidBodyConstructionInfo);
	m_body->setAngularFactor(VEC3_TO_BTVEC3(m_lookRotation));
    m_body->setUserPointer(reinterpret_cast<void *>(entity->getGameObject()));

    if (fMotionThreshold >= 0.0f)
        m_body->setCcdMotionThreshold(fMotionThreshold);

    if (fSweptSphereRadius >= 0.0f)
        m_body->setCcdSweptSphereRadius(fSweptSphereRadius);

    PhysicsScene::AddRigidbody(&(*m_body));
}

void Rigidbody::shutdown() {
    if (!m_body) {
        return;
    }

    PhysicsScene::RemoveRigidbody(&(*m_body));
}

void Rigidbody::update() {
    if (!m_body || !m_transform) {
        return;
    }

    if (getGameObject()->isActive()) {
        if (m_body->getLinearVelocity().length() < 0.01 && m_body->getAngularVelocity().length() < 0.01) {
            m_body->setActivationState(DISABLE_DEACTIVATION);
        } else {
            m_body->forceActivationState(ACTIVE_TAG);
        }
	}

	if (m_body->isActive()) {
		btTransform trans;
		m_body->getMotionState()->getWorldTransform(trans);

		m_transform->setLocalPosition({trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()});

		m_transform->setLocalRotation({trans.getRotation().getW(), trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ()});
	}
}

float Rigidbody::getMass() const {
    return m_mass;
}

void Rigidbody::setMass(float value) {
    m_mass = value;
}

void Rigidbody::setLookRotation(const Vector3 &value) {
    m_lookRotation = value;

    if (m_body)
        m_body->setAngularFactor(VEC3_TO_BTVEC3(value));
}

const Vector3 &Rigidbody::getLookRotation() const {
    return m_lookRotation;
}

float Rigidbody::getMotionThreshold() const {
    return fMotionThreshold;
}

void Rigidbody::setMotionThreshold(float value) {
    fMotionThreshold = value;
}

float Rigidbody::getSweptSphereRadius() const {
    return fSweptSphereRadius;
}

void Rigidbody::setSweptSphereRadius(float value) {
    fSweptSphereRadius = value;
}
