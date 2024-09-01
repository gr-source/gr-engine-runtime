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
    };

    static std::vector<GameObject*> s_objects;

    static grr::gShader *m_shader[3];

    static Camera *m_camera;

    static bool bRender;

    static void SetupLight();

    static void SetupMaterial(std::unordered_map<int, Material> &materials);

    static bool SetupShader(void *shader, const std::string &path);
};
