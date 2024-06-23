#pragma once

#include "common.h"

#include <gr-render/gShader.h>

class RenderManager {
public:
    static void OnRenderer(grr::gShader *shader);

    static void ModelFromRender(GameObject *gameObject);
private:
    static std::unordered_map<std::string, GameObject*> models;
};
