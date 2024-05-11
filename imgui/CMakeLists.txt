project("ImGui")

set(imgui_src
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_glfw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui.cpp
)

set(imgui_hdr
    backends/imgui_impl_opengl3_loader.h
    backends/imgui_impl_opengl3.h
    backends/imgui_impl_glfw.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imgui_internal.h
    imconfig.h
    imgui.h
)

add_library(imgui STATIC ${imgui_src} ${imgui_hdr})
