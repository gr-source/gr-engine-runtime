#include "renderManager.h"

#include "gameObject.h"

#include "component/MeshRender.h"
#include "component/meshFilter.h"

void RenderManager::OnRenderer(grr::gShader* shader) {
    auto& objects = Renderer::objects;
    
    if (objects.empty()) {
        return;
    }

    std::for_each(objects.begin(), objects.end(), ModelFromRender);
}

void RenderManager::ModelFromRender(GameObject *gameObject) {
    auto& transform = gameObject->getTransform();

    grr::gShader::SetUniform("model", 1, transform.getglobalmatrix());

    auto& meshRender = gameObject->getComponent<MeshRender>();

    meshRender.draw();
}
