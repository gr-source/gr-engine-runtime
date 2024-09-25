project("script")

set(sourc_src
    gr-script.cpp
    #{GR_SRC_START}
    #{GR_SRC_END}
)

set(sourc_hdr
    gr-script.hpp
    #{GR_HDR_START}
    #{GR_HDR_END}
)

add_library(${PROJECT_NAME} STATIC ${sourc_src} ${sourc_hdr})

