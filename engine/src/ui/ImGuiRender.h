#pragma once

// #include <imgui_impl_opengl3.h>
// #include <imgui_impl_glfw.h>
#include <imgui.h>

class ImGuiRender {
public:

    ImGuiRender();

    ImGuiRender(const char* openglVersion);

    void PushStyle(ImGuiStyle& dst);

    void addFont(const char* path);

    void Begin();

    void End();

    ~ImGuiRender();
protected:
    ImGuiIO* io;
};


