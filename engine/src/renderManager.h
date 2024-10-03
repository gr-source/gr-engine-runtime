#pragma once

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

    static void SetupLight();

    static void SetupShader(ShaderType type, grr::gShader *shader);

    static void RegisterUniform(bool model);
};
