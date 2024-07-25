#include "renderManager.h"

#include "objectManager.h"
#include "gameObject.h"

#include "component/cameraComponent.h"
#include "component/lightComponent.h"
#include "component/meshRender.hpp"
#include "component/Transform.h"

#include "component/camera.hpp"

#include "resources/resources.h"
#include "resources/shader.h"

void RenderManager::Initialize() {
    auto& models = MeshRender::GetGameObjectFromRender();
    for (const auto& gameObject : models) {
        std::cout << gameObject->getName() << std::endl;
    }
}

void RenderManager::Process() {
}

void RenderManager::Renderer() {
}
