#pragma once

#include "culling/frustum.h"

#include <gr-render/gRender.h>

#include "geometry/material.h"

#include "default.hpp"
#include "common.h"

class RenderManager {
public:
    static void Initialize();

    static void Process();

    static void Renderer();

    static void Shutdown();

    static bool bWireFrame;
    
    static Frustum s_frustum;

public:
    static std::vector<GameObject*> s_objects;

    static grr::gShader *m_shader;

    static Camera *m_camera;


    static void Filter(GameObject *gameObject, MeshRender *meshRender);
    
    static void SetupLight();

    static void SetupMaterial(std::unordered_map<int, Material> &materials);
};
