project("gr-render")

set(gr_render_src
<<<<<<< HEAD
    src/gTexture.cpp
    src/gRender.cpp
    src/gShader.cpp
=======
    src/grender.cpp
    src/gbuffer.cpp
    src/common.cpp
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
    src/gl.cpp
)

set(gr_render_hdr
<<<<<<< HEAD
    src/gTexture.h
    src/gRender.h
    src/gShader.h
    src/common.h
    src/gColor.h
    src/types.h
=======
    src/grender.h
    src/gbuffer.h
    src/common.h
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
    src/gl.h
)

add_library(render STATIC ${gr_render_src} ${gr_render_hdr})

find_package(GLEW REQUIRED)

target_include_directories(render PRIVATE ${GLEW_INCLUDE_DIR})


