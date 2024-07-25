project("engine")

set(guizmo_src
    src/guizmo/ImGuizmo.cpp
)

set(guizmo_hdr
    src/guizmo/ImGuizmo.h
)

set(ecs_src
    src/ecs/entity.cpp
)

set(ecs_hdr
    src/ecs/entity.h
    src/ecs/component.h
)

set(octree_src
    src/octree/octree.cpp
)

set(octree_hdr
    src/octree/octree.h
)

set(simplexNoise_src
    src/simplexNoise/SimplexNoise.cpp
)

set(simplexNoise_hdr
    src/simplexNoise/SimplexNoise.h
)

set(scriptManager_cpp
    src/scriptManager/scriptManager.cpp
)

set(scriptManager_hpp
    src/scriptManager/scriptManager.hpp
)

set(engine_src
    ${guizmo_src}
    ${octree_src}
    ${simplexNoise_src}
    ${scriptManager_cpp}

    src/gl.cpp
    src/log.cpp
    src/timer.cpp
    src/Game.cpp
    src/input.cpp
    src/engine.cpp
    src/memory.cpp
    src/Global.cpp
    src/skybox.cpp
    src/screen.cpp
    src/renderer.cpp
    src/settings.cpp
    src/gameObject.cpp
    src/screenRender.cpp
    src/eventManager.cpp
    src/chunkManager.cpp
    src/renderManager.cpp
    src/objectManager.cpp
    src/bufferManager.cpp
    src/editorController.cpp

    src/component/camera.cpp
    src/component/Transform.cpp
    src/component/meshRender.cpp
    src/component/meshFilter.cpp
    src/component/lightComponent.cpp
    src/component/cameraComponent.cpp
    src/component/cameraController.cpp

    src/framebuffer/gBuffer.cpp
    src/framebuffer/thumbnails.cpp
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
    
    src/animation/bone.cpp
    src/animation/keyframe.cpp
    src/animation/skeleton.cpp
    src/animation/animation.cpp
    src/animation/animationClip.cpp

    src/filmbox/fbxMaterial.cpp
    src/filmbox/fbxCamera.cpp
    src/filmbox/fbxLight.cpp
    src/filmbox/fbxScene.cpp
    src/filmbox/fbxNode.cpp
    src/filmbox/fbxMesh.cpp
    src/filmbox/Filmbox.cpp

    src/geometry/material.cpp
    src/geometry/model.cpp
    src/geometry/mesh.cpp
    
    src/core/scene/Scene.cpp
    src/core/scene/SceneManager.cpp
    
    ${ecs_src}

    src/ui/View.cpp
    src/ui/ImGuiRender.cpp

    src/culling/frustum.cpp

    src/filesystem/filesystem.cpp
    src/filesystem/stream.cpp

    src/physics/physicsScene.cpp
)

set(engine_hdr
    ${guizmo_hdr}
    ${octree_hdr}
    ${simplexNoise_hdr}
    ${scriptManager_hpp}
    
    src/log.h
    src/timer.h
    src/Game.h
    src/input.h
    src/engine.h
    src/memory.h
    src/Global.h
    src/skybox.h
    src/screen.h
    src/renderer.h
    src/settings.h
    src/gameObject.h
    src/screenRender.h
    src/eventManager.h
    src/chunkManager.h
    src/renderManager.h
    src/objectManager.h
    src/bufferManager.h
    src/editorController.h

    src/component/camera.hpp
    src/component/default.hpp
    src/component/Transform.h
    src/component/meshRender.hpp
    src/component/meshFilter.hpp
    src/component/lightComponent.h
    src/component/cameraComponent.h
    src/component/cameraController.h

    src/framebuffer/gBuffer.h
    src/framebuffer/thumbnails.h
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

    src/animation/bone.h
    src/animation/keyframe.h
    src/animation/skeleton.h
    src/animation/animation.h
    src/animation/animationClip.h

    src/filmbox/fbxMaterial.h
    src/filmbox/fbxCamera.h
    src/filmbox/fbxLight.h
    src/filmbox/fbxScene.h
    src/filmbox/fbxNode.h
    src/filmbox/fbxMesh.h
    src/filmbox/Filmbox.h

    src/geometry/material.h
    src/geometry/model.h
    src/geometry/mesh.h

    src/core/scene/Scene.h
    src/core/scene/SceneManager.h

    ${ecs_hdr}

    src/ui/View.h
    src/ui/ImGuiRender.h

    src/culling/frustum.h
    
    src/filesystem/filesystem.h
    src/filesystem/stream.h

    src/physics/physicsScene.h
)

add_library(engine STATIC ${engine_src} ${engine_hdr})


