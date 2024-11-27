#include "script/rigidbody.hpp"

#include "script/transform.hpp"

#include "core/function.hpp"
#include "core/stream.hpp"

#include <bullet/btBulletDynamicsCommon.h>

static void (*RigidbodyInitialize)(Rigidbody *instance);
INSERT_FUNCTION(0, RigidbodyInitialize)

static void (*RigidbodyUpdate)(Rigidbody *instance);
INSERT_FUNCTION(1, RigidbodyUpdate)

static void (*RigidbodyShutdown)(Rigidbody *instance);
INSERT_FUNCTION(2, RigidbodyShutdown)

Rigidbody::Rigidbody() : m_collisionShape(nullptr), m_DefaultMotionState(nullptr), m_body(nullptr), m_transform(nullptr), m_mass(0.0f), m_bDinamic(false), m_lookRotation(Vector3::one), fMotionThreshold(0.0f), fSweptSphereRadius(0.0f) {}

Rigidbody::~Rigidbody() {
    RigidbodyShutdown(this);
}

void Rigidbody::Init() {
    m_transform = GetComponent<Transform>();

    RigidbodyInitialize(this);
}

void Rigidbody::Update() {
    // RigidbodyUpdate(this);
}

bool Rigidbody::Serialize(Stream *stream) {
    stream->write(&m_mass, sizeof(float));
    stream->write(&m_bDinamic, sizeof(bool));
    stream->write(&m_lookRotation, sizeof(Vector3));
    stream->write(&fMotionThreshold, sizeof(float));
    stream->write(&fSweptSphereRadius, sizeof(float));
    return true;
}

bool Rigidbody::Deserialize(Stream *stream) {
    stream->read(&m_mass, sizeof(float));
    stream->read(&m_bDinamic, sizeof(bool));
    stream->read(&m_lookRotation, sizeof(Vector3));
    stream->read(&fMotionThreshold, sizeof(float));
    stream->read(&fSweptSphereRadius, sizeof(float));
    return true;
}

Vector3 Rigidbody::getLinearVelocity() const {
    return {m_body->getLinearVelocity().getX(), m_body->getLinearVelocity().getY(), m_body->getLinearVelocity().getZ()};
}

void Rigidbody::setLinearVelocity(const Vector3 &value) {
    m_body->setLinearVelocity({value.x, value.y, value.z});
}

void Rigidbody::applyCentralForce(const Vector3 &value) {
    m_body->applyCentralForce({value.x, value.y, value.z});
}

void Rigidbody::applyCentralImpulse(const Vector3 &value) {
    m_body->applyCentralImpulse({value.x, value.y, value.z});
}

void Rigidbody::applyCentralPushImpulse(const Vector3 &value) {
    m_body->applyCentralPushImpulse({value.x, value.y, value.z});
}

void Rigidbody::applyForce(const Vector3 &force, const Vector3 &pos) {
    m_body->applyForce({force.x, force.y, force.z}, {pos.x, pos.y, pos.z});
}

void Rigidbody::applyImpulse(const Vector3 &impulse, const Vector3 &pos) {
    m_body->applyImpulse({impulse.x, impulse.y, impulse.z}, {pos.x, pos.y, pos.z});
}

void Rigidbody::applyPushImpulse(const Vector3 &impulse, const Vector3 &pos) {
    m_body->applyPushImpulse({impulse.x, impulse.y, impulse.z}, {pos.x, pos.y, pos.z});
}

void Rigidbody::applyTorque(const Vector3 &value) {
    m_body->applyTorque({value.x, value.y, value.z});
}

void Rigidbody::applyTorqueImpulse(const Vector3 &value) {
    m_body->applyTorqueImpulse({value.x, value.y, value.z});
}

void Rigidbody::applyTorqueTurnImpulse(const Vector3 &value) {
    m_body->applyTorqueTurnImpulse({value.x, value.y, value.z});
}

float Rigidbody::getMass() const {
    return m_mass;
}

void Rigidbody::setMass(float value) {
    m_mass = value;
}

void Rigidbody::setDynamic(bool value) {
    m_bDinamic = value;
}

btRigidBody *Rigidbody::getBody() const {
    return &(*m_body);
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

void Rigidbody::setPosition(const Vector3 &position) {
    auto &transform = m_body->getWorldTransform();
    transform.setOrigin({position.x, position.y, position.z});
}

Vector3 Rigidbody::getPosition() const {
    auto &transform = m_body->getWorldTransform();

    auto origin = transform.getOrigin();
    
    return {origin.getX(), origin.getY(), origin.getZ()};
}

void Rigidbody::setRotation(Quaternion q) {
    auto &transform = m_body->getWorldTransform();
    transform.setRotation({q.x, q.y, q.z, q.w});
}

Quaternion Rigidbody::getRotation() const {
    auto &transform = m_body->getWorldTransform();

    return {transform.getRotation().getW(), transform.getRotation().getX(), transform.getRotation().getY(), transform.getRotation().getZ()};
}

void Rigidbody::setColissionFlags(int flags) {
    m_body->setCollisionFlags(flags);
}

int Rigidbody::getColissionFlags() const {
    return m_body->getCollisionFlags();
}
