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

    const Color& getcolor() const;

    void setcolor(const Color& color);

    void setType(int value);

    int getType() const;

    const float& getattenuationconstant();

    const float& getattenuationlinear();
    
    const float& getattenuationquadractc();

    const float& getinnecone();

    const float& getoutercone();

private:
    int m_type;

    Transform* m_transform;

    Color m_color;

    float m_attenuationConstant;

    float m_attenuationLinear;

    float m_attenuationQuadratic;

    float m_angleInnerCone;

    float m_angleOuterCone;
};


