#pragma once

#include "../common.h"

#include "ecs/component.hpp"

class Light : public Component {
public:
    enum LightType : int {
        eDirection = 0,
        ePoint     = 1,
        eSpot      = 2,
    };

    Light() = delete;

    Vector3 getposition() const;

    Vector3 getForward() const;

    Color getcolor() const;

    void setcolor(const Color& color);

    void setType(int value);

    int getType() const;

    float getAttenuationConstant() const;

    void setAttenuationConstant(float value);

    float getAttenuationLinear() const;

    void setAttenuationLinear(float value);
    
    float getAttenuationQuadractc() const;

    void setAttenuationQuadractc(float value);

    float getInnecone() const;

    void setInnecone(float value);

    float getOutercone() const;

    void setOutercone(float value);

    Matrix4x4 getProjetion() const;

private:
    static Matrix4x4 mProjection;

    int m_type;

    Transform* m_transform;

    Color m_color;

    float m_attenuationConstant;

    float m_attenuationLinear;

    float m_attenuationQuadratic;

    float m_angleInnerCone;

    float m_angleOuterCone;
};


