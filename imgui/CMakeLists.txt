project("imgui")

set(imgui_src
    src/backends/imgui_impl_opengl3.cpp
    src/backends/imgui_impl_glfw.cpp
    src/texteditor/textEditor.cpp
    src/imgui_widgets.cpp
    src/imgui_tables.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui.cpp
)

add_library(${PROJECT_NAME} SHARED ${imgui_src} ${imgui_hdr})
