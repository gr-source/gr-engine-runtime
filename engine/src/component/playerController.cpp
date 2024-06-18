#include "playerController.h"

#include "cameraController.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "animator.h"

#include "../screen.h"
#include "../input.h"
#include "../timer.h"

#include "../objectManager.h"

PlayerController::PlayerController() : m_transform(nullptr), m_rigidbody(nullptr), m_animator(nullptr), m_cameraController(nullptr) {}

void PlayerController::OnInit() {
    m_transform = &entity->getComponent<Transform>();
    m_rigidbody = &entity->getComponent<Rigidbody>();
    m_animator = &entity->getComponent<Animator>();
}

void PlayerController::OnStart() {
    GameObject* camera = ObjectManager::find("Camera");
    if (camera != nullptr) {
        m_camera = &camera->getTransform();
    }
}

void PlayerController::OnUpdate() {
    float velocity = (pPlayerSpeed * 100.0f * Time::deltaTime);
 
    Vector3 direction;

    const Vector3 cameraDirection = Math::normalize(Vector3(m_camera->getforward().x, 0.0f, m_camera->getforward().z));
    // if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W)) {
    //     direction += cameraDirection * velocity;
    // }
    // if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S)) {
    //     direction -= cameraDirection * velocity;
    // }
    // if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A)) {
    //     direction += Math::normalize(m_camera->getright()) * velocity;
    // }
    // if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D)) {
    //     direction -= Math::normalize(m_camera->getright()) * velocity;
    // }
    direction.y += PhysicsScene::gravity.y;
 
    m_rigidbody->addlinearvelocity(direction);

    // if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE)) {
    //     m_rigidbody->addlinearvelocity(Vector3::up * velocity);
    // }
}

