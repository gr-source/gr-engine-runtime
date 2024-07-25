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
};
