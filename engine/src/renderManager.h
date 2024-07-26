#pragma once

#include "common.h"

#include "culling/frustum.h"

#include <gr-render/gRender.h>
#include <gr-render/gShader.h>

class RenderManager {
public:
    
    static void Initialize();

    static void Process();

    static void Renderer();

public:
    static Frustum s_frustum;

    static std::vector<GameObject*> s_objects;

    static grr::gShader *m_shader;
};
