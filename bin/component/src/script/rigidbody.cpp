#include "script/rigidbody.hpp"

#include "script/transform.hpp"

#include "core/stream.hpp"

#include <bullet/btBulletDynamicsCommon.h>

Rigidbody::Rigidbody() : m_collisionShape(nullptr), m_DefaultMotionState(nullptr), m_body(nullptr), m_transform(nullptr), m_mass(0.0f), m_bDinamic(false), m_lookRotation(Vector3::one) {}

Rigidbody::~Rigidbody() {
    shutdown();
}

void Rigidbody::Init() {
    m_transform = &GetComponent<Transform>();

    initialize();
}

void Rigidbody::Update() {
    update();
}

bool Rigidbody::Serialize(Stream *stream) {
    stream->write(&m_mass, sizeof(float));
    stream->write(&m_bDinamic, sizeof(bool));
    stream->write(&m_lookRotation, sizeof(Vector3));
    return true;
}

bool Rigidbody::Deserialize(Stream *stream) {
    stream->read(&m_mass, sizeof(float));
    stream->read(&m_bDinamic, sizeof(bool));
    stream->read(&m_lookRotation, sizeof(Vector3));
    return true;
}

Vector3 Rigidbody::getLinearVelocity() const {
    return {m_body->getLinearVelocity().getX(), m_body->getLinearVelocity().getY(), m_body->getLinearVelocity().getZ()};
}

void Rigidbody::setLinearVelocity(const Vector3 &value) {
    m_body->setLinearVelocity({value.x, value.y, value.z});
}

void Rigidbody::applyCentralImpulse(const Vector3 &value) {
    m_body->applyCentralImpulse({value.x, value.y, value.z});
}

void Rigidbody::update() {
    throw std::runtime_error("function do not implemented.");
}

void Rigidbody::shutdown() {
}

void Rigidbody::initialize() {
    throw std::runtime_error("function do not implemented.");
}
