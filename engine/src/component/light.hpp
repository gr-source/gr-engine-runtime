#pragma once

#include "../ecs/entity.h"

#include "../common.h"

class Light : public Component {
public:
    enum : u8 {
        eDirection = 1,
        ePoint     = 2,
        eSpot      = 3,
    };

    static std::vector<Light *> &GetListLight();

    const Vector3& getposition() const;

    const Vector3 getforward() const;

    const Color& getcolor() const;

    void setcolor(const Color& color);

    const int& gettype() const;

    const float& getattenuationconstant();

    const float& getattenuationlinear();
    
    const float& getattenuationquadractc();

    const float& getinnecone();

    const float& getoutercone();

protected:
    int m_type;

    Transform* m_transform;

    Color m_color;

    float m_attenuationConstant;

    float m_attenuationLinear;

    float m_attenuationQuadratic;

    float m_angleInnerCone;

    float m_angleOuterCone;
};


