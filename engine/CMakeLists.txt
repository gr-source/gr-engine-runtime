project("engine")

set(guizmo_src
    src/guizmo/ImGuizmo.cpp
)

set(guizmo_hdr
    src/guizmo/ImGuizmo.h
)

set(ecs_src
    src/ecs/gameObject.cpp
    src/ecs/component.cpp
    src/ecs/entity.cpp
)

set(ecs_hdr
    src/ecs/gameObject.h
    src/ecs/component.h
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
)

set(scriptManager_hdr
    src/scriptManager/scriptManager.hpp
)

set(core_src
    src/core/thread.cpp
    src/core/scene.cpp
)

set(core_hdr
    src/core/thread.hpp
    src/core/scene.hpp
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

set(engine_src
    ${core_src}
    ${guizmo_src}
    ${octree_src}
    ${filmbox_src}
    ${animation_src}
    ${simplexNoise_src}
    ${scriptManager_cpp}

    src/gl.cpp
    src/log.cpp
    src/Game.cpp
    src/input.cpp
    src/timer.cpp
    src/engine.cpp
    src/memory.cpp
    src/Global.cpp
    src/skybox.cpp
    src/screen.cpp
    src/console.cpp
    src/renderer.cpp
    src/settings.cpp
    src/screenRender.cpp
    src/eventManager.cpp
    src/renderManager.cpp
    src/bufferManager.cpp
    src/postProcessing.cpp
    src/editorController.cpp

    src/component/light.cpp
    src/component/camera.cpp
    src/component/animator.cpp
    src/component/transform.cpp
    src/component/meshRender.cpp
    src/component/meshFilter.cpp

    src/framebuffer/imguibuffer.cpp
    src/framebuffer/screenbuffer.cpp
    src/framebuffer/shadowbuffer.cpp
    src/framebuffer/skyboxbuffer.cpp
    src/framebuffer/internal/framebuffer.cpp
    src/framebuffer/internal/renderbuffer.cpp
    
    src/resources/shader.cpp
    src/resources/cubemap.cpp
    src/resources/texture.cpp
    src/resources/resources.cpp
    
    src/geometry/material.cpp
    src/geometry/vertex3D.cpp
    src/geometry/model.cpp
    src/geometry/mesh.cpp
    
    ${ecs_src}

    src/ui/View.cpp
    src/ui/ImGuiRender.cpp

    src/culling/frustum.cpp

    src/filesystem/filesystem.cpp
    src/filesystem/stream.cpp

    src/physics/physicsScene.cpp
)

set(engine_hdr
    ${core_hdr}
    ${guizmo_hdr}
    ${octree_hdr}
    ${filmbox_hdr}
    ${animation_hdr}
    ${scriptManager_hdr}

    src/log.h
    src/timer.h
    src/Game.h
    src/input.h
    src/engine.h
    src/memory.h
    src/Global.h
    src/skybox.h
    src/screen.h
    src/console.hpp
    src/renderer.h
    src/settings.h
    src/screenRender.hpp
    src/eventManager.h
    src/renderManager.h
    src/bufferManager.h
    src/postProcessing.hpp
    src/editorController.h
    
    src/component/light.hpp
    src/component/camera.hpp
    src/component/default.hpp
    src/component/transform.hpp
    src/component/meshRender.hpp
    src/component/meshFilter.hpp

    src/framebuffer/imguibuffer.h
    src/framebuffer/screenbuffer.h
    src/framebuffer/shadowbuffer.h
    src/framebuffer/skyboxbuffer.h
    src/framebuffer/internal/framebuffer.h
    src/framebuffer/internal/renderbuffer.h
    
    src/resources/shader.h
    src/resources/cubemap.h
    src/resources/texture.h
    src/resources/resources.h
    
    src/geometry/material.h
    src/geometry/vertex3D.h
    src/geometry/model.h
    src/geometry/mesh.h
    
    ${ecs_hdr}
    
    src/ui/View.h
    src/ui/ImGuiRender.h

    src/culling/frustum.h
    
    src/filesystem/filesystem.h
    src/filesystem/stream.h

    src/physics/physicsScene.h
)

add_library(engine STATIC ${engine_src} ${engine_hdr})

# target_include_directories(engine PRIVATE
#     $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
# )

