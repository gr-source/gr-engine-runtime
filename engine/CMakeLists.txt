project(engine VERSION 0.1.0 LANGUAGES C CXX)


set(guizmo_src
    src/guizmo/ImGuizmo.cpp
)

set(guizmo_hdr
    src/guizmo/ImGuizmo.h
)

set(ecs_src
    src/ecs/gameObject.cpp
    src/ecs/component.cpp
    src/ecs/manager.cpp
    src/ecs/entity.cpp
)

set(ecs_hdr
    src/ecs/gameObject.h
    src/ecs/component.h
    src/ecs/manager.hpp
    src/ecs/entity.h
)

set(octree_src
    src/octree/octree.cpp
)

set(octree_hdr
    src/octree/octree.h
)

set(scriptManager_cpp
    src/scriptManager/scriptManager.cpp
    src/scriptManager/scriptLoader.cpp
)

set(scriptManager_hdr
    src/scriptManager/scriptManager.hpp
    src/scriptManager/scriptLoader.hpp
)

set(core_src
    src/core/assetsManager.cpp
    src/core/statistics.cpp
    src/core/threadpool.cpp
    src/core/console.cpp
    src/core/string.cpp
    src/core/screen.cpp
    src/core/event.cpp
    src/core/scene.cpp
    src/core/input.cpp
    src/core/time.cpp
    src/core/timer.cpp
)

set(core_hdr
    src/core/assetsManager.hpp
    src/core/statistics.hpp
    src/core/threadpool.hpp
    src/core/console.hpp
    src/core/string.hpp
    src/core/screen.hpp
    src/core/event.hpp
    src/core/scene.hpp
    src/core/input.hpp
    src/core/time.hpp
    src/core/timer.hpp
)

set(filmbox_src
    src/core/filmbox/fbxAnimation.cpp
    src/core/filmbox/fbxMaterial.cpp
    src/core/filmbox/fbxChannel.cpp
    src/core/filmbox/fbxVertex.cpp
    src/core/filmbox/fbxScene.cpp
    src/core/filmbox/fbxBone.cpp
    src/core/filmbox/fbxMesh.cpp
    src/core/filmbox/fbxNode.cpp
    src/core/filmbox/filmbox.cpp
)

set(filmbox_hdr
    src/core/filmbox/fbxAnimation.hpp
    src/core/filmbox/fbxMaterial.hpp
    src/core/filmbox/fbxChannel.hpp
    src/core/filmbox/fbxVertex.hpp
    src/core/filmbox/fbxScene.hpp
    src/core/filmbox/fbxBone.hpp
    src/core/filmbox/fbxMesh.hpp
    src/core/filmbox/fbxNode.hpp
    src/core/filmbox/filmbox.hpp
)

set(animation_src
    src/core/animation/animation.cpp
    src/core/animation/keyframe.cpp
    src/core/animation/bone.cpp
)

set(animation_hdr
    src/core/animation/boneWeight.cpp
    src/core/animation/animation.cpp
    src/core/animation/keyframe.cpp
    src/core/animation/bone.cpp
)

set(core_ecs_src
    src/core/ecs/componentPool.cpp
)

set(core_ecs_hdr
    src/core/ecs/componentPool.hpp
)

set(core_file_src
    src/core/file/initialization.cpp
    src/core/file/stream.cpp
    src/core/file/config.cpp
    src/core/file/file.cpp
)

set(core_file_hdr
    src/core/file/initialization.hpp
    src/core/file/stream.hpp
    src/core/file/config.hpp
    src/core/file/file.hpp
)

set(core_cmd_hdr
    src/core/cmd/cmd.hpp
)

set(core_cmd_src
    src/core/cmd/cmd.cpp
)

set(engine_src
    ${core_src}
    ${guizmo_src}
    ${octree_src}
    ${filmbox_src}
    ${core_ecs_src}
    ${core_cmd_src}
    ${animation_src}
    ${core_file_src}
    ${simplexNoise_src}
    ${scriptManager_cpp}
    
    src/engine.cpp
    src/memory.cpp
    src/Global.cpp
    src/skybox.cpp
    src/renderer.cpp
    src/settings.cpp
    src/screenRender.cpp
    src/renderManager.cpp
    src/postProcessing.cpp
    src/cubeFramebuffer.cpp
    src/editorController.cpp

    src/script/meshCollider.cpp
    src/script/boxCollider.cpp
    src/script/meshFilter.cpp
    src/script/meshRender.cpp
    src/script/rigidbody.cpp
    src/script/transform.cpp
    src/script/animator.cpp
    src/script/camera.cpp
    src/script/light.cpp

    src/geometry/batching.cpp
    src/geometry/boundbox.cpp
    src/geometry/material.cpp
    src/geometry/vertex3D.cpp
    src/geometry/mesh.cpp
    
    ${ecs_src}

    src/ui/View.cpp
    src/ui/editor.cpp
    src/ui/project.cpp
    src/ui/ImGuiRender.cpp
    src/ui/projectMenu.cpp

    src/culling/frustum.cpp

    src/physics/physicsScene.cpp

    src/core/binary/gModel3D.cpp
    src/core/binary/gHeader.cpp
)

set(engine_hdr
    ${core_hdr}
    ${guizmo_hdr}
    ${octree_hdr}
    ${filmbox_hdr}
    ${core_ecs_hdr}
    ${core_cmd_hdr}
    ${animation_hdr}
    ${core_file_hdr}
    ${scriptManager_hdr}

    src/engine.h
    src/memory.h
    src/Global.h
    src/skybox.h
    src/renderer.h
    src/settings.h
    src/screenRender.hpp
    src/renderManager.h
    src/postProcessing.hpp
    src/editorController.h
    src/cubeFramebuffer.hpp
    
    src/script/meshCollider.hpp
    src/script/boxCollider.hpp
    src/script/meshRender.hpp
    src/script/meshFilter.hpp
    src/script/transform.hpp
    src/script/camera.hpp
    src/script/light.hpp

    src/geometry/batching.hpp
    src/geometry/boundbox.h
    src/geometry/material.h
    src/geometry/vertex3D.h
    src/geometry/mesh.h
    
    ${ecs_hdr}
    
    src/ui/View.h
    src/ui/editor.hpp
    src/ui/project.hpp
    src/ui/ImGuiRender.h
    src/ui/projectMenu.hpp

    src/culling/frustum.h
    
    src/physics/physicsScene.hpp

    src/core/binary/gModel3D.hpp
    src/core/binary/gHeader.hpp
)

add_library(${PROJECT_NAME} STATIC ${engine_src} ${engine_hdr})

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src>
)


