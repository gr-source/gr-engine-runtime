#include "rigidbody.hpp"

#include "meshCollider.hpp"

#include "physics/physicsScene.hpp"

#include "script/boxCollider.hpp"
#include "script/transform.hpp"

#include "console.hpp"

#define FROMVEC3(vec) btVector3(vec.x, vec.y, vec.z)
#define FROMQUAT(q) btQuaternion(q.x, q.y, q.z, q.w)

void Rigidbody::setup() {
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

    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
    m_collisionShape->calculateLocalInertia(m_mass, fallInertia);

    btRigidBody::btRigidBodyConstructionInfo pRigidBodyConstructionInfo(m_mass, &(*m_DefaultMotionState), m_collisionShape, fallInertia);
    pRigidBodyConstructionInfo.m_friction = 1.0f;
    pRigidBodyConstructionInfo.m_restitution = 0.3f;

    m_body = std::make_shared<btRigidBody>(pRigidBodyConstructionInfo);
	// m_body->setAngularFactor(FROMVEC3(def.loockRotation));

    PhysicsScene::AddRigidbody(&(*m_body));
}

void Rigidbody::process() {
    if (m_bDinamic) {
        if (m_body->getLinearVelocity().length() < 0.01 && m_body->getAngularVelocity().length() < 0.01) {
            m_body->setActivationState(DISABLE_DEACTIVATION); // Ativa o objeto
        } else {
            m_body->forceActivationState(ACTIVE_TAG); // Mantém o objeto ativo
        }
	}

	if (m_body->isActive() && m_mass > 0.0f) {
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
    if (value <= 0.0f) {
        // Pode desativar ou fazer algo apropriado
        return;
    }

    m_body->clearForces();
    m_body->activate(true); // Ativa o corpo

    // Calcular a inércia local e a massa
    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
    m_collisionShape->calculateLocalInertia(value, fallInertia);
    
    // Atualizar propriedades de massa e inércia
    m_body->setMassProps(value, fallInertia);
    m_body->setInvInertiaDiagLocal(fallInertia);
    
    // Garantir que o corpo está ativo
    m_body->setActivationState(ACTIVE_TAG);
    
    // Aplique uma força para verificar o movimento
    m_body->applyCentralForce(btVector3(0, 10, 0));
    
    m_mass = value; // Atualizar a massa armazenada
}

bool Rigidbody::getDinamic() {
    return m_bDinamic;
}

void Rigidbody::setDynamic(bool value) {
    m_bDinamic = value;
}


