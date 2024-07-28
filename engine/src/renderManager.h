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

    static bool bWireFrame;

    static int iDrawCount;

    static int iDrawRender;

public:
    static std::vector<GameObject*> s_objects;

    static grr::gShader *m_shader;

    static Frustum s_frustum;
    
    static void SetupLight();

    static void SetupMaterial(std::unordered_map<int, Material> &materials);
};
