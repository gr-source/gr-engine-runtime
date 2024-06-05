#pragma once

#include "common.h"

#include "geometry/material.h"

class RenderManager {
public:
    
    static void Initialize();

    static void Process();

    static void Renderer();

    static u32 drawBatchinCount;

    static u32 drawCallRender;
    static u32 drawCallCount;
    static u32 drawCount;

private:
    static Shader* m_shaderModel;

    static void SetupMaterial(Shader* shader, std::unordered_map<u32, Material>& materials);
};
