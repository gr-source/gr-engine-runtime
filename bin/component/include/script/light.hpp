#pragma once

#include <ecs/entity.hpp>

#include "default.hpp"

class Light : public Component {
public:
    enum LightType : int {
        eDirection = 0,
        ePoint     = 1,
        eSpot      = 2
    };

    static std::vector<Light *> m_lights;

    Light(std::uint8_t type = eDirection, const Color& color = Color::white, float aConstant = 1.0f, float aLinear = 1.0f, float aQuadract = 1.0f, float inneCone = 1.0f, float outerCone = 1.0f);

    void Init() override;

    void Destroy() override;

    void setColor(const Color &color);

    void Enable() override;

    void Disable() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

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

REGISTER_COMPONENT(Light);



