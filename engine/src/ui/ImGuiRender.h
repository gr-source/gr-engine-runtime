#pragma once

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
#include <imgui.h>

class ImGuiRender {
public:
    static void Initialize(const char* openglVersion);
    
    static void Shutdown();

    static void PushStyle(ImGuiStyle& dst);

    static void AddFont(const char* path);

    static void Begin();

    static void End();

protected:
    static ImGuiIO* s_io;
};


