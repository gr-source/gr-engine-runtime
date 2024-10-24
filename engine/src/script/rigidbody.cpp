#include "rigidbody.hpp"

#include "meshCollider.hpp"

#include "physics/physicsScene.hpp"

#include "script/boxCollider.hpp"
#include "script/transform.hpp"

#include "console.hpp"

#define FROMVEC3(vec) btVector3(vec.x, vec.y, vec.z)
#define FROMQUAT(q) btQuaternion(q.x, q.y, q.z, q.w)

void Rigidbody::initialize() {
    if (entity->hasComponent<MeshCollider>()) {
        m_collisionShape = entity->getComponent<MeshCollider>().getShape();
    } else if (entity->hasComponent<BoxCollider>()) {
        m_collisionShape = entity->getComponent<BoxCollider>().getShape();
    }

    if (!m_collisionShape) {
        return LOG_ERROR("[ Rigidbody ] Shape not found.");
    }

    m_DefaultMotionState = std::make_shared<btDefaultMotionState>(
		btTransform(
			FROMQUAT(m_transform->getLocalRotation()),
			FROMVEC3(m_transform->getLocalPosition())
		)
	);

    // if (m_mass <= 0.0f) {
    //     m_mass = 1.0f;
    // }

    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
    m_collisionShape->calculateLocalInertia(m_mass, fallInertia);

    btRigidBody::btRigidBodyConstructionInfo pRigidBodyConstructionInfo(m_mass, &(*m_DefaultMotionState), m_collisionShape, fallInertia);
    pRigidBodyConstructionInfo.m_friction = 1.0f;
    pRigidBodyConstructionInfo.m_restitution = 0.3f;

    m_body = std::make_shared<btRigidBody>(pRigidBodyConstructionInfo);
	m_body->setAngularFactor(FROMVEC3(m_lookRotation));

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

    if (m_bDinamic) {
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

bool Rigidbody::getDinamic() const {
    return m_bDinamic;
}

void Rigidbody::setDynamic(bool value) {
    m_bDinamic = value;
}

void Rigidbody::setLookRotation(const Vector3 &value) {
    m_lookRotation = value;

    if (m_body)
        m_body->setAngularFactor(FROMVEC3(value));
}

const Vector3 &Rigidbody::getLookRotation() const {
    return m_lookRotation;
}
