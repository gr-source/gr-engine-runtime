cmake_minimum_required(VERSION 3.5.0)

project(gr-component VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)

set(ecs_src
    src/ecs/gameObject.cpp
    src/ecs/entity.cpp
)

set(script_src
    src/script/meshCollider.cpp
    src/script/boxCollider.cpp
    src/script/meshRender.cpp
    src/script/meshFilter.cpp
    src/script/rigidbody.cpp
    src/script/transform.cpp
    src/script/animator.cpp
    src/script/camera.cpp
    src/script/light.cpp
)

set(core_src
    src/core/assetsManager.cpp
    src/core/physicsScene.cpp
    src/core/threadpool.cpp
    src/core/function.cpp
    src/core/console.cpp
    src/core/raycast.cpp
    src/core/stream.cpp
    src/core/screen.cpp
    src/core/string.cpp
    src/core/event.cpp
    src/core/scene.cpp
    src/core/input.cpp
    src/core/file.cpp
    src/core/time.cpp
    src/core/draw.cpp
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

add_library(${PROJECT_NAME} SHARED main.cpp src/api.cpp ${ecs_src} ${script_src} ${source_src} ${core_src} ${animation_src} ${geometry_src})

target_link_libraries(${PROJECT_NAME} PRIVATE ${GRMATH_LIBRARIES} ${BULLET_LIBRARIES} /home/grdev/Documentos/gr-engine/gr-base/lib/libgr-base.so)

target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS}
    /home/grdev/Documentos/gr-engine/gr-base/include/gr-base
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

