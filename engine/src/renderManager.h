#pragma once

#include "common.h"

#include <gr-render/gShader.h>

class RenderManager {
public:
    static void OnRenderer(grr::gShader *shader, bool canReadMaterial);

    static void ModelFromRender(GameObject *gameObject);
private:
    static std::unordered_map<std::string, GameObject*> models;

    static bool readyMaterial;

    static void SetuMaterial(MeshRender &meshRender);
};
