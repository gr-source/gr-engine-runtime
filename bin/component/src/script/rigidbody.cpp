#include "script/rigidbody.hpp"

#include "script/transform.hpp"

Rigidbody::Rigidbody() : m_DefaultMotionState(nullptr), m_body(nullptr), m_transform(nullptr), m_mass(0.0f), m_bDinamic(false) {}

void Rigidbody::Init() {
    m_transform = &GetComponent<Transform>();

    setup();
}

void Rigidbody::Update() {
    process();
}

void Rigidbody::process() {
    throw std::runtime_error("function do not implemented.");
}

void Rigidbody::setup() {
    throw std::runtime_error("function do not implemented.");
}
