cmake_minimum_required(VERSION 3.31.5)

project(gr-component VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)

set(core_src
    src/core/function.cpp
    src/core/raycast.cpp
    src/core/time.cpp
    src/core/draw.cpp
)

set(source_src
    source/gr-script.cpp
    #{GR_SRC_START}
    #{GR_SRC_END}
)

set(source_hdr
    source/gr-script.hpp
    #{GR_HDR_START}
    #{GR_HDR_END}
)

find_package(gr-math REQUIRED)

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

add_library(${PROJECT_NAME} SHARED main.cpp src/api.cpp ${source_src} ${core_src})

target_link_libraries(${PROJECT_NAME} PRIVATE ${GRMATH_LIBRARIES} ${BULLET_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS}
    /home/felipy/Documentos/gr-projects/gr-base/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE REGISTRY)
