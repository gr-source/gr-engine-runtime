#pragma once

#include "common.h"

#include "geometry/material.h"

#include "culling/frustum.h"

#include "octree/octree.h"

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
    static std::vector<GameObject*> m_objects;

    static std::unique_ptr<Octree> m_octree;

    static Shader* m_shaderModel;

    static Frustum m_frustum;

    static void SetupMaterial(Shader* shader, std::unordered_map<u32, Material>& materials);
    
    static void SetupLight(Shader* shader);
};
