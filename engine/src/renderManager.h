#pragma once

#include <gr-render/gRender.h>

#include <gr-base/core/geometry/material.hpp>

#include "default.hpp"
#include "common.h"
#include "gr-base/core/culling/octreemanager.hpp"

class RenderManager {
public:
    static void Process(const Camera *camera);

    static void Renderer();

public:
    static bool CheckNodeOnCamFrustum(OctreeNode *node);

    static void SetupLight(grr::gShader *shader);
};
