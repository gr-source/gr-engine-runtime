#include "view.hpp"

const bool View::TextureInputUI(const ImColor& color) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 size = ImGui::GetItemRectSize();

    // Define as coordenadas do retângulo
    ImVec2 rectMin = ImGui::GetCursorScreenPos();
    ImVec2 rectMax = ImVec2(rectMin.x + size.x + 100, rectMin.y + size.y);

    // Adiciona o retângulo à lista de desenho
    drawList->AddRectFilled(rectMin, rectMax, color);

    return ImGui::InvisibleButton(std::string("#TextureInputUI" + std::to_string(color.Value.x)).c_str(), ImVec2(rectMax.x - rectMin.x, size.y));
}

const bool View::PopupChangeColor(const std::string& name, Color& color) {
    if (ImGui::BeginPopup(std::string("#PopupChangeColor" + name).c_str())) {
        auto isOpen = ImGui::ColorPicker4(name.c_str(), color.data);
        ImGui::EndPopup();
        return isOpen;
    }
    return false;
}

bool View::DragScalarEx(const char *name[], float *data, int size, float v_speed, float p_min, float p_max) {
    bool value_changed = false;

    ImGui::BeginGroup();
    ImGui::PushID(data);

    ImGui::PushMultiItemsWidths(size, ImGui::CalcItemWidth());
    for (int i=0;i<size;i++) {
        char buffer[256];
        sprintf(buffer, "##%s_%d", name[i], i);

        ImGui::PushID(i);

        ImGui::Text("%s", name[i]);

        ImGui::SameLine();

        value_changed |= ImGui::DragFloat(buffer, &data[i], v_speed, p_min, p_max);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::PopID();
    }
    ImGui::PopID();
    ImGui::EndGroup();
    return value_changed;
}

bool View::ImageTreeNode(ImTextureID textureID, const char *name) {
    const ImGuiStyle& style = ImGui::GetStyle();
    ImGuiStorage* storage = ImGui::GetStateStorage();

    ImGui::PushID(name);

    // ImU32 id = ImGui::GetID(name);

    bool opened = false;//storage->GetInt(id, 0);
    float x = ImGui::GetCursorPosX();

    // ImGui::ArrowButton

    // float cross_extent = ImGui::GetFontSize() * 0.5f * 0.7071f - 1.0f;

    ImGui::BeginGroup();
    opened = ImGui::ArrowButton(name, ImGuiDir_Right);
    // opened = ImGui::ImageButton(textureID, {16, 16});

    ImGui::SameLine();

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    if (hovered || active)
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

    ImGui::Text("%s", name);
    ImGui::EndGroup();
    if (opened)
        ImGui::TreePush(name);
    ImGui::PopID();
    return opened;
}

bool View::CloseButton() {
    return false;
}
