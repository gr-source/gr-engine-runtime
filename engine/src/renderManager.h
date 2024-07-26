#pragma once

#include "common.h"

#include "culling/frustum.h"

#include <gr-render/gRender.h>
#include <gr-render/gShader.h>

#include "geometry/material.h"

class RenderManager {
public:
    
    static void Initialize();

    static void Process();

    static void Renderer();

    static void Shutdown();

public:
    static Frustum s_frustum;

    static std::vector<GameObject*> s_objects;

    static grr::gShader *m_shader;

    static void SetupLight();

    static void SetupMaterial(const std::unordered_map<int, Material> &materials);
};
