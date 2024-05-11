#include "cameraComponent.h"

#include "Transform.h"

#include "../objectManager.h"
#include "../screen.h"
#include "../time.h"

#include "../core/scene/SceneManager.h"

std::vector<CameraComponent*> CameraComponent::m_cameras;
u32 CameraComponent::CameraID = 0;

CameraComponent &CameraComponent::MainCamera() {
    if (m_cameras.empty()) {
        GameObject& m_mainCamera = ObjectManager::add("MainCamera");

        Transform& m_transform = m_mainCamera.getComponent<Transform>();
        m_transform.position = {-22.098204f, 19.242348f, -27.375128f};

        SceneManager::Add(&m_mainCamera);

        return m_mainCamera.addComponent<CameraComponent>(45.0f, 0.001f, 500.0f, (float(Screen::width / Screen::height)));
    }
    return *m_cameras[CameraID];
}

CameraComponent::CameraComponent(float fov, float nearPlane, float farPlane, float aspect) : m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane), m_aspect(aspect) {
    m_cameras.push_back(this);
}

void CameraComponent::OnInit() {
    m_transform = &entity->getComponent<Transform>();
}

const Vector3 &CameraComponent::getposition() {
    return m_transform->position;
}

const Matrix4x4 CameraComponent::getViewMatrix() const {
    return Math::lookAt(m_transform->position, m_transform->position + m_transform->getforward(), m_transform->getup());
}

const Matrix4x4 CameraComponent::getPerspesctive() const {
    // return Math::orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 75.0f, 0.001f);
    return Math::perspective(Math::rad(m_fov), m_aspect, m_farPlane, m_nearPlane);
}

Transform *CameraComponent::gettransform() {
    return m_transform;
}

void CameraComponent::SetFildOfView(float fov) {
    if (fov < 1.0f) return;

    m_fov = fov;
}

void CameraComponent::OnGui() {
    if (ImGui::TreeNode("CameraComponent")) {
        ImGui::Text("Fov:");
        ImGui::DragFloat("##CameraComponentFov", &m_fov, 1.0f, 1.0f, 179.0f);

        ImGui::Text("FarPlane:");
        ImGui::DragFloat("##CameraComponentFarPlane", &m_farPlane);

        ImGui::Text("NearPlane:");
        ImGui::DragFloat("##CameraComponentNearPlane", &m_nearPlane);

        ImGui::TreePop();
    }
}

CameraComponent::~CameraComponent() {
    m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), this), m_cameras.end());
}

