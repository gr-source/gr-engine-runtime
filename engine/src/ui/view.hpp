#pragma once

#include "ImGuiRender.h"

#include <gr-math/gmath.h>
#include <string>

class View {
public:
    static const bool TextureInputUI(const ImColor& color);

    static const bool PopupChangeColor(const std::string& name, Color& color);

    static bool DragScalarEx(const char *name[], float *data, int size, float v_speed, float p_min, float p_max);

    static bool ImageTreeNode(ImTextureID textureID, const char *name);

    static bool CloseButton();
};






