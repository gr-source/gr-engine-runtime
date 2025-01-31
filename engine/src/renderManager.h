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
    enum ShaderType : int {
        ShaderType_Dynamic = 0,
        ShaderType_Dynamic_Anim = 1,
        ShaderType_Dynamic_Multi_Model = 2,
        ShaderType_Static_Multi_Model = 3,
    };

    static std::array<grr::gShader *, 4> m_shader;

    static Camera *m_camera;

    static bool bRender;

    static void SetupLight(grr::gShader *shader);

    static bool SetupShader(ShaderType type, grr::gShader *shader);

    static void RegisterUniform(bool model, grr::gShader *shader);
};
