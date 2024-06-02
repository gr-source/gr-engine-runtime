project("gr-render")

set(gr_render_src
    src/gTexture.cpp
    src/gRender.cpp
    src/gShader.cpp
    src/gl.cpp
)

set(gr_render_hdr
    src/gTexture.h
    src/gRender.h
    src/gShader.h
    src/common.h
    src/gColor.h
    src/types.h
    src/gl.h
)

add_library(render STATIC ${gr_render_src} ${gr_render_hdr})

find_package(GLEW REQUIRED)

target_include_directories(render PRIVATE ${GLEW_INCLUDE_DIR})


