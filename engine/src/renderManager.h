#pragma once

#include "common.h"

class RenderManager {
public:
    
    static void Initialize();

    static void Process();

    static void Renderer();

    static u32 drawBatchinCount;

    static u32 drawCallCount;

    static u32 drawCount;

private:
    static Shader* m_shaderModel;
};
