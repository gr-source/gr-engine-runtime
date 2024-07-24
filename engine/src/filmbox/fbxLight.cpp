#include "fbxLight.h"

FbxLight::FbxLight(const std::string name, LightType type, const Color &color, float attenuationConstant, float attenuationLinear, float attenuationQuadratic, const Vector3& forward, const Vector3& up, float angleInnerCone, float angleOuterCone) : m_name(name), m_type(type), m_color(color), m_attenuationConstant(attenuationConstant), m_attenuationLinear(attenuationLinear), m_attenuationQuadratic(attenuationQuadratic), m_forward(forward), m_up(up), m_angleInnerCone(angleInnerCone), m_angleOuterCone(angleOuterCone) {
}

const FbxLight::LightType &FbxLight::gettype() const {
    return m_type;
}

const Vector3 &FbxLight::getforward() const {
    return m_forward;
}

const Vector3 &FbxLight::getup() const {
    return m_up;
}

const Color &FbxLight::getcolor() const {
    return m_color;
}

const float &FbxLight::getAttenuationConstant() const {
    return m_attenuationConstant;
}

const float &FbxLight::getAttenuationLinear() const {
    return m_attenuationLinear;
}

const float &FbxLight::getAttenuationQuadract() const {
    return m_attenuationQuadratic;
}

const float &FbxLight::getInneCone() const {
    return m_angleInnerCone;
}

const float &FbxLight::getOuterCone() const {
    return m_angleOuterCone;
}
