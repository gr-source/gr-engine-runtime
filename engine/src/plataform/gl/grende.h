#pragma once

#include <string>

enum RenderState {
    GR_FALSE = 0,
    GR_TRUE = 1,
    GR_BACKGROUND_COLOR = 2,
    GR_BACKGROUND = 3,
    GR_DEPTH_BUFFER = 4,
    GR_COLOR_BUFFER = 5,
};

void SetRenderState(RenderState state, void* value);

void SetRenderState(RenderState state, RenderState value);

const std::string getRenderStateName(RenderState state);


