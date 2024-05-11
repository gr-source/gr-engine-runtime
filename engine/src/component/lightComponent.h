#pragma once

#include "../ecs/Entity.h"

class LightComponent : public Component {
public:
    enum : u8 {
        eDirection = 1,
        ePoint     = 2,
        eSpot      = 3,
    };

    static std::vector<LightComponent*> m_lights;

    LightComponent(u8 type, const Color& color = Color::white, float aConstant = 1.0f, float aLinear = 1.0f, float aQuadract = 1.0f, float inneCone = 1.0f, float outerCone = 1.0f);
    ~LightComponent();

    void OnUpdate() override;

    const Vector3& getposition() const;

    const Vector3 getforward() const;

    const Color& getcolor() const;

    void setcolor(const Color& color);

    const int& gettype() const;

    void OnInit() override;

    void OnGui() override;

    const float& getattenuationconstant();

    const float& getattenuationlinear();
    
    const float& getattenuationquadractc();

    const float& getinnecone();

    const float& getoutercone();

    const Matrix4x4 getprojectionview() const;

protected:
    int m_type;

    Transform* m_transform;

    Color m_color;

    float m_attenuationConstant;

    float m_attenuationLinear;

    float m_attenuationQuadratic;

    float m_angleInnerCone;

    float m_angleOuterCone;

    static Matrix4x4 projection;
};