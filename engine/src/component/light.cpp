#include "light.hpp"

#include "transform.hpp"

const Vector3 Light::getposition() const {
    return m_transform->getPosition();
}

const Vector3 Light::getforward() const {
    return m_transform->getforward();
}

const Color &Light::getcolor() const {
    return m_color;
}

void Light::setcolor(const Color &color) {
    m_color = color;
}

const int &Light::gettype() const {
    return m_type;
}

const float &Light::getattenuationconstant() {
    return m_attenuationConstant;
}

const float &Light::getattenuationlinear() {
    return m_attenuationLinear;
}

const float &Light::getattenuationquadractc() {
    return m_attenuationQuadratic;
}

const float &Light::getinnecone() {
    return m_angleInnerCone;
}

const float &Light::getoutercone() {
    return m_angleOuterCone;
}


