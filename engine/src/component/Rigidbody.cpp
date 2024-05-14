#include "Rigidbody.h"

#include "Transform.h"
#include "boxCollider.h"

#define FROMVEC3(vec) btVector3(vec.x, vec.y, vec.z)
#define FROMQUAT(q) btQuaternion(q.x, q.y, q.z, q.w)

Rigidbody::Rigidbody(const RigidbodyDef& def) : pPhysicsScene(&PhysicsScene::getInstance()), def(def), pDefaultMotionState(nullptr), body(nullptr) {}

Rigidbody::~Rigidbody() {
}

void Rigidbody::OnInit() {
	m_transform = &entity->getComponent<Transform>();
	m_collider = &entity->getComponent<Collider>();

    pDefaultMotionState = std::make_unique<btDefaultMotionState>(
		btTransform(
			FROMQUAT(m_transform->rotation),
			FROMVEC3(m_transform->position)
		)
	);

    btVector3 fallInertia(0, 0, 0);
    m_collider->getshape()->calculateLocalInertia(def.mass, fallInertia);

    btRigidBody::btRigidBodyConstructionInfo pRigidBodyConstructionInfo(def.mass, pDefaultMotionState.get(), m_collider->getshape(), fallInertia);
    pRigidBodyConstructionInfo.m_friction = def.friction;
    pRigidBodyConstructionInfo.m_restitution = def.restitution;

    body = std::make_unique<btRigidBody>(pRigidBodyConstructionInfo);

	body->setAngularFactor(FROMVEC3(def.loockRotation));

	std::lock_guard<std::mutex> lock(m_mutex);
    pPhysicsScene->addRigidbody(body.get());
}

void Rigidbody::OnStart() {
	std::lock_guard<std::mutex> lock(m_mutex);

	btVector3 fallInertia(0, 0, 0);
    m_collider->getshape()->calculateLocalInertia(def.mass, fallInertia);

	body->setFriction(def.friction);
	body->setRestitution(def.restitution);

	body->setInvInertiaDiagLocal(fallInertia);

	btTransform trans;
	trans.setOrigin(FROMVEC3(m_transform->position));
	trans.setRotation(FROMQUAT(m_transform->rotation));
	
	body->setWorldTransform(trans);

	body->clearForces();
}

void Rigidbody::OnUpdate() {
	std::lock_guard<std::mutex> lock(m_mutex);

	if (def.isDynamic) {
		if (body->getLinearVelocity().length() < 0.01 && body->getAngularVelocity().length() < 0.01) {
			body->setActivationState(DISABLE_DEACTIVATION); // Ativa o objeto
		} else {
			body->forceActivationState(ACTIVE_TAG); // Mantém o objeto ativo
		}
	}

	if (body->isActive()) {
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);

		m_transform->position = {trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()};

		m_transform->rotation = {trans.getRotation().getW(), trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ()};

		// if (m_transform->hasparent()) {
		// 	m_transform->OnUpdateParents();
		// }
	}
}

void Rigidbody::addimpulse(const Vector3& impulse) {
	std::lock_guard<std::mutex> lock(m_mutex);
	body->applyImpulse(FROMVEC3(impulse), btVector3(0.0f, 0.0f, 0.0f));
}

void Rigidbody::addcentralforce(const Vector3 &force) {
	std::lock_guard<std::mutex> lock(m_mutex);
	body->applyCentralForce(FROMVEC3(force));
}

void Rigidbody::addlinearvelocity(const Vector3 &velocity) {
	std::lock_guard<std::mutex> lock(m_mutex);
	body->setLinearVelocity(FROMVEC3(velocity));
}

void Rigidbody::GetAABB(Vector3& min, Vector3& max) {
	btVector3 mi, ma;
    body->getAabb(mi, ma);
	min = {mi.getX(), mi.getY(), mi.getZ()};
	max = {ma.getX(), ma.getY(), ma.getZ()};
}
