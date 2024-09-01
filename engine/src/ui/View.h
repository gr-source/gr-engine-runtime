#pragma once

#include "ImGuiRender.h"

#include <gr-math/gmath.h>
#include <string>

class View {
public:
    static const bool TextureInputUI(const ImColor& color);

    static const bool PopupChangeColor(const std::string& name, Color& color);

    static const bool DragScalarEx(const char* id, const char* label[], ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);

    static bool ImageTreeNode(ImTextureID textureID, const char *name);

    static bool CloseButton();
};






