#pragma once

#include <string>

#include "../Color.h"

#include <gr-math/vector3.h>

class FbxLight {
public:
    enum LightType {
        eNONE = 0,
        eDIRECTION = 1,
        ePOINT = 2,
        eSPOT = 3,
    };

    FbxLight(const std::string name = "default", LightType type = eNONE, const Color& color = Color::white, float attenuationConstant = 0.0f, float attenuationLinear = 0.0f, float attenuationQuadratic = 0.0f, const Vector3& forward = Vector3::forward, const Vector3& up = Vector3::up, float angleInnerCone = 45.0f, float angleOuterCone = 10.0f);

    const LightType& gettype() const;

    const Vector3& getforward() const;

    const Vector3& getup() const;

    const Color& getcolor() const;

    const float& getAttenuationConstant() const;

    const float& getAttenuationLinear() const;

    const float& getAttenuationQuadract() const;

    const float& getInneCone() const;

    const float& getOuterCone() const;

private:
    std::string m_name;

    LightType m_type;

    Vector3 m_forward;

    Vector3 m_up;

    Color m_color;

    float m_attenuationConstant;

    float m_attenuationLinear;

    float m_attenuationQuadratic;

    float m_angleInnerCone;

    float m_angleOuterCone;
};


