#include "lightComponent.h"

#include "Transform.h"

#include "../renderer.h"

std::vector<LightComponent*> LightComponent::m_lights;
Matrix4x4 LightComponent::projection = Math::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 75.0f);


LightComponent::LightComponent(u8 type, const Color& color, float aConstant, float aLinear, float aQuadract, float inneCone, float outerCone) : m_type(type), m_color(color), m_attenuationConstant(aConstant), m_attenuationLinear(aLinear), m_attenuationQuadratic(aQuadract), m_transform(nullptr), m_angleInnerCone(inneCone), m_angleOuterCone(outerCone) {
    m_lights.push_back(this);
}

LightComponent::~LightComponent() {
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), this), m_lights.end());
}

void LightComponent::Update() {
    DebugDraw::addLine(m_transform->position, (m_transform->position + m_transform->getforward()), m_color);
    DebugDraw::addPoint(m_transform->position, Color::red);
}

void LightComponent::Init() {
    // m_transform = &entity->getComponent<Transform>();
}

const Vector3 &LightComponent::getposition() const {
    return m_transform->position;
}

const Vector3 LightComponent::getforward() const {
    return m_transform->getforward();
}

const Color &LightComponent::getcolor() const {
    return m_color;
}

void LightComponent::setcolor(const Color &color) {
    m_color = color;
}

const int &LightComponent::gettype() const {
    return m_type;
}

const float &LightComponent::getattenuationconstant() {
    return m_attenuationConstant;
}

const float &LightComponent::getattenuationlinear() {
    return m_attenuationLinear;
}

const float &LightComponent::getattenuationquadractc() {
    return m_attenuationQuadratic;
}

const float &LightComponent::getinnecone() {
    return m_angleInnerCone;
}

const float &LightComponent::getoutercone() {
    return m_angleOuterCone;
}

const Matrix4x4 LightComponent::getprojectionview() const {
    return projection * Math::lookAt(m_transform->position, m_transform->position + m_transform->getforward(), m_transform->getup());
}
