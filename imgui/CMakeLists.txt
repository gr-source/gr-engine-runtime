project("imgui")

set(imgui_src
    src/backends/imgui_impl_gr.cpp
    src/imgui_widgets.cpp
    src/imgui_tables.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui.cpp
)

set(imgui_hdr
    src/backends/imgui_impl_gr.h
    src/imstb_rectpack.h
    src/imstb_textedit.h
    src/imstb_truetype.h
    src/imgui_internal.h
    src/imconfig.h
    src/imgui.h
)

add_library(imgui STATIC ${imgui_src} ${imgui_hdr})
