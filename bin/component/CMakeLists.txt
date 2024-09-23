cmake_minimum_required(VERSION 3.5.0)
project(gr-component VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)

set(CMAKE_CXX_STANDARD 17)

set(ecs_src
    src/ecs/componentRegistry.cpp
    src/ecs/gameObject.cpp
    src/ecs/component.cpp
    src/ecs/entity.cpp
    src/ecs/field.cpp
)

set(script_src
    src/script/meshRender.cpp
    src/script/meshFilter.cpp
    src/script/transform.cpp
    # src/script/animator.cpp
    src/script/camera.cpp
    src/script/light.cpp
)

set(core_src
    src/core/assetsManager.cpp
    src/core/console.cpp
    src/core/stream.cpp
    src/core/screen.cpp
    src/core/string.cpp
    src/core/event.cpp
    src/core/scene.cpp
    src/core/input.cpp
    src/core/file.cpp
    src/core/time.cpp
)

set(geometry_src
    src/core/geometry/material.cpp
    src/core/geometry/boundBox.cpp
    src/core/geometry/vertex3D.cpp
    src/core/geometry/batching.cpp
    src/core/geometry/mesh.cpp
)

set(animation_src
    src/core/animation/boneWeight.cpp
    src/core/animation/animation.cpp
    src/core/animation/keyframe.cpp
    src/core/animation/bone.cpp
)

find_package(gr-math REQUIRED)

add_library(${PROJECT_NAME} STATIC main.cpp src/api.cpp ${ecs_src} ${script_src} ${core_src} ${animation_src} ${geometry_src})

target_link_libraries(${PROJECT_NAME} PRIVATE ${GRMATH_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
