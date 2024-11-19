#include "light.hpp"

#include "transform.hpp"

#include <glm/glm.hpp>


Matrix4x4 Light::mProjection = Math::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 75.0f);
// -10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f

Vector3 Light::getPosition() const {
    return m_transform->getPosition();
}

Vector3 Light::getForward() const {
    return m_transform->getForward();
}

Color Light::getColor() const {
    return m_color;
}

void Light::setColor(const Color &color) {
    m_color = color;
}

void Light::setType(int value) {
    m_type = value;
}

int Light::getType() const {
    return m_type;
}

float Light::getAttenuationConstant() const {
    return m_attenuationConstant;
}

void Light::setAttenuationConstant(float value) {
    m_attenuationConstant = value;
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

#include <glm/ext.hpp>

Matrix4x4 Light::getProjetion() const {
    // auto p = m_transform->getPosition();
    // auto l = m_transform->getPosition() + m_transform->getforward();
    // auto f = m_transform->getup();

    // float near_plane = 0.1f, far_plane = 7.50f;
    // glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); 


    // glm::mat4 lightView = glm::lookAt(glm::vec3(p.x, p.y, p.z), 
    //                                 glm::vec3(0, 0, 0), 
    //                                 glm::vec3(0, 1, 0));  

    // glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // return *reinterpret_cast<Matrix4x4 *>(glm::value_ptr(lightSpaceMatrix));
    return mProjection * Math::lookAt(m_transform->getPosition(), m_transform->getPosition() + m_transform->getForward(), m_transform->getUp());
}






