#include "View.h"

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

const bool View::DragScalarEx(const char* id, const char* label[], ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
    return false;
}
