#pragma once

#include <list>

#include "../Global.h"

namespace View {
    struct Item {
        const char* name;
        ImVec4 color;

        Item(const char* name, const ImVec4& color) : name(name), color(color) {}
    };

    bool TextureInputUI(const ImColor& color);

    void PopupChangeColor(const std::string& name, Color& color);
}