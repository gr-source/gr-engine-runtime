#pragma once

#include "common.h"

#include <map>
#include <vector>
#include <functional>

#include "geometry/material.h"
#include "geometry/mesh.h"

#include "culling/frustum.h"

class BatchingManager {
public:
    static void Initialize();

    static void OnUpdate();

    static void OnRender();

    static void OnRenderShadow();

    /* shader deffred */
    // static void OnRenderLight(GBuffer* gbuffer);

    static u32 frustomDraw;
    static u32 drawCount;

protected:
    static Skybox* m_skybox;

    static Shader* m_shader;

    static Shader* m_shaderLights;
    
    static Frustum m_frustum;

    static CameraComponent* m_cameraComponent;

    static Shadowbuffer* m_shadowbuffer;

    static std::vector<GameObject*> m_objectrender;

    static void RenderModel(Shader* shader);

    static void SetupMaterial(const MeshRender& meshRender, Shader* shader);

    static void SetupLight(Shader* shader);
};
