project("gr-render")

set(gr_render_src
    src/grender.cpp
    src/gbuffer.cpp
    src/common.cpp
    src/gl.cpp
)

set(gr_render_hdr
    src/grender.h
    src/gbuffer.h
    src/common.h
    src/gl.h
)

add_library(render STATIC ${gr_render_src} ${gr_render_hdr})

find_package(GLEW REQUIRED)

target_include_directories(render PRIVATE ${GLEW_INCLUDE_DIR})


