#include "renderManager.h"

#include "gameObject.h"

#include "component/MeshRender.h"
#include "component/meshFilter.h"

#include <gr-render/gColor.h>

bool RenderManager::readyMaterial = false;

void RenderManager::OnRenderer(grr::gShader* shader, bool canReadMaterial) {
    auto& objects = Renderer::objects;

    if (objects.empty()) {
        return;
    }

    readyMaterial = canReadMaterial;

    std::for_each(objects.begin(), objects.end(), ModelFromRender);
}

void RenderManager::ModelFromRender(GameObject *gameObject) {
    auto& transform = gameObject->getTransform();

    if (readyMaterial) {
        auto& meshRender = gameObject->getComponent<MeshRender>();

        SetuMaterial(meshRender);
    }

    grr::gShader::SetUniform("model", 1, transform.getglobalmatrix());

    auto& meshRender = gameObject->getComponent<MeshRender>();

    meshRender.draw();
}

void RenderManager::SetuMaterial(MeshRender &meshRender) {
    auto& materials = meshRender.m_materials;

    for (u16 i=0;i<materials.size();i++) {
        grr::gShader::SetUniform(("materials[" + std::to_string(i) + "].albedo").c_str(), 1, *reinterpret_cast<grr::gColor*>(&materials[i].albedo));
        grr::gShader::SetUniform(("materials[" + std::to_string(i) + "].params").c_str(), 1, Vector3(
            materials[i].roughness,
            materials[i].metallic,
            materials[i].ao
        ));
    }
}
