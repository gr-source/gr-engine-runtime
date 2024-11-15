#include "light.hpp"

#include "transform.hpp"

Matrix4x4 Light::mProjection = Math::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 75.0f);
 // -10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f
 
Vector3 Light::getposition() const {
    return m_transform->getPosition();
}

Vector3 Light::getForward() const {
    return m_transform->getforward();
}

Color Light::getcolor() const {
    return m_color;
}

void Light::setcolor(const Color &color) {
    m_color = color;
}

void Light::setType(int value) {
    m_type = value;
}

int Light::getType() const {
    return m_type;
}

void Light::setAttenuationConstant(float value) {
    m_attenuationConstant = value;
}

float Light::getAttenuationConstant() const {
    return m_attenuationConstant;
}

float Light::getAttenuationLinear() const {
    return m_attenuationLinear;
}

void Light::setAttenuationLinear(float value) {
    m_attenuationLinear = value;
}

float Light::getAttenuationQuadractc() const {
    return m_attenuationQuadratic;
}

void Light::setAttenuationQuadractc(float value) {
    m_attenuationQuadratic = value;
}

float Light::getInnecone() const {
    return m_angleInnerCone;
}

void Light::setInnecone(float value) {
    m_angleInnerCone = value;
}

float Light::getOutercone() const {
    return m_angleOuterCone;
}

void Light::setOutercone(float value) {
    m_angleOuterCone = value;
}

Matrix4x4 Light::getProjetion() const {
    return mProjection * Math::lookAt(m_transform->getPosition(), m_transform->getPosition() + m_transform->getforward(), m_transform->getup());
}
