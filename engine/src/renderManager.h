#pragma once

#include <gr-render/gRender.h>

#include <gr-base/core/geometry/material.hpp>

#include "default.hpp"
#include "common.h"

class RenderManager {
public:
    static void Process(const Camera *camera);

    static void Renderer();

public:
    static void SetupLight(grr::gShader *shader);
};
