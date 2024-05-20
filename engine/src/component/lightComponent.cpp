#include "lightComponent.h"

#include "Transform.h"

std::vector<LightComponent*> LightComponent::m_lights;
Matrix4x4 LightComponent::projection = Math::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);


LightComponent::LightComponent(u8 type, const Color& color, float aConstant, float aLinear, float aQuadract, float inneCone, float outerCone) : m_type(type), m_color(color), m_attenuationConstant(aConstant), m_attenuationLinear(aLinear), m_attenuationQuadratic(aQuadract), m_transform(nullptr), m_angleInnerCone(inneCone), m_angleOuterCone(outerCone) {
    m_lights.push_back(this);
    active = true;
}

LightComponent::~LightComponent() {
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), this), m_lights.end());
}

void LightComponent::OnUpdate() {
    DebugDraw::addLine(m_transform->position, (m_transform->position + m_transform->getforward()), m_color);
    DebugDraw::addPoint(m_transform->position, Color::red);
}

void LightComponent::OnInit() {
    m_transform = &entity->getComponent<Transform>();
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

/*
void LightComponent::OnGui() {
    if (ImGui::TreeNode("LightComponent")) {
        ImGui::Text("Constante:");
        ImGui::DragFloat("##constant", &m_attenuationConstant, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Linear:");
        ImGui::DragFloat("##linear", &m_attenuationLinear, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Quadract:");
        ImGui::DragFloat("##quadract", &m_attenuationQuadratic, 0.005f, 0.01f, 1.0f);

        ImGui::Text("InConner:");
        ImGui::DragFloat("##inconer", &m_angleInnerCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("IuterConner:");
        ImGui::DragFloat("##outerconner", &m_angleOuterCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Cor:");
        ImVec4 color = {m_color.r / 255, m_color.g / 255, m_color.b / 255, 1.0f};
        if (ImGui::ColorButton("##LightColor", color)) {
            ImGui::OpenPopup("##LightColorPopup");
        }
        if (ImGui::BeginPopup("##LightColorPopup")) {
            ImGui::ColorPicker4("#LightColorPick", m_color.data);
            ImGui::EndPopup();
        }
        ImGui::TreePop();
    }
}
*/

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
