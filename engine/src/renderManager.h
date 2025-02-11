#pragma once

#include <gr-render/gRender.h>

#include <gr-base/core/geometry/material.hpp>

#include "default.hpp"
#include "common.h"

class RenderManager {
public:
    static void Initialize();

    static void Process(Camera *camera, ShadowMap *shadowMap);

    static void Renderer();

    static void Shutdown();

    static bool bWireFrame;
    
public:
    static void SetupLight(grr::gShader *shader);
};
