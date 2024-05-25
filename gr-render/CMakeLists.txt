project("gr-render")

set(gr_render_src
    src/grender.cpp
)

set(gr_render_hdr
    src/grender.h
)

add_library(render STATIC ${gr_render_src} ${gr_render_hdr})

find_package(GLEW REQUIRED)

target_include_directories(render PRIVATE ${GLEW_INCLUDE_DIR})


