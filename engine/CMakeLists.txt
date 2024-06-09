project("engine")

set(guizmo_src
    src/guizmo/ImGuizmo.cpp
)

set(guizmo_hdr
    src/guizmo/ImGuizmo.h
)

set(ecs_src
    src/ecs/entity.cpp
    src/ecs/entityManager.cpp
)

set(ecs_hdr
    src/ecs/entity.h
    src/ecs/component.h
    src/ecs/entityManager.h
)

set(octree_src
    src/octree/octree.cpp
)

set(octree_hdr
    src/octree/octree.h
)

set(engine_src
    ${guizmo_src}
    ${octree_src}

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
    src/thread.cpp
    src/renderer.cpp
    src/settings.cpp
    src/gameObject.cpp
    src/screenRender.cpp
    src/eventManager.cpp
    src/renderManager.cpp
    src/objectManager.cpp
    src/bufferManager.cpp
    src/editorController.cpp

    src/component/animator.cpp
    src/component/collider.cpp
    src/component/Transform.cpp
    src/component/Rigidbody.cpp
    src/component/MeshRender.cpp
    src/component/meshFilter.cpp
    src/component/boxCollider.cpp
    src/component/meshCollider.cpp
    src/component/lightComponent.cpp
    src/component/cameraComponent.cpp
    src/component/playerController.cpp
    src/component/cameraController.cpp
    src/component/characterController.cpp

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

    src/filmbox/fbxNode.cpp
    src/filmbox/fbxMesh.cpp
    src/filmbox/Filmbox.cpp
    src/filmbox/fbxScene.cpp
    src/filmbox/fbxLight.cpp
    src/filmbox/fbxCamera.cpp

    src/geometry/batchedGeometry.cpp
    src/geometry/geometryRender.cpp
    src/geometry/material.cpp
    src/geometry/model.cpp
    src/geometry/mesh.cpp
    
    src/core/scene/Scene.cpp
    src/core/scene/SceneManager.cpp
    
    ${ecs_src}

    src/ui/View.cpp
    src/ui/ImGuiRender.cpp

    src/culling/occlusion.cpp
    src/culling/frustum.cpp

    src/physics/physicsScene.cpp

    src/filesystem/filesystem.cpp
    src/filesystem/stream.cpp
)

set(engine_hdr
    ${guizmo_hdr}
    ${octree_hdr}
    
    src/log.h
    src/timer.h
    src/Game.h
    src/input.h
    src/engine.h
    src/memory.h
    src/Global.h
    src/skybox.h
    src/screen.h
    src/thread.h
    src/renderer.h
    src/settings.h
    src/gameObject.h
    src/screenRender.h
    src/eventManager.h
    src/renderManager.h
    src/objectManager.h
    src/bufferManager.h
    src/editorController.h

    src/component/animator.h
    src/component/collider.h
    src/component/Transform.h
    src/component/Rigidbody.h
    src/component/MeshRender.h
    src/component/meshFilter.h
    src/component/boxCollider.h
    src/component/meshCollider.h
    src/component/lightComponent.h
    src/component/cameraComponent.h
    src/component/playerController.h
    src/component/cameraController.h
    src/component/characterController.h

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

    src/filmbox/fbxNode.h
    src/filmbox/fbxMesh.h
    src/filmbox/Filmbox.h
    src/filmbox/fbxScene.h
    src/filmbox/fbxLight.h
    src/filmbox/fbxCamera.h

    src/geometry/batchedGeometry.h
    src/geometry/geometryRender.h
    src/geometry/material.h
    src/geometry/model.h
    src/geometry/mesh.h

    src/core/scene/Scene.h
    src/core/scene/SceneManager.h

    ${ecs_hdr}

    src/ui/View.h
    src/ui/ImGuiRender.h

    src/culling/occlusion.h
    src/culling/frustum.h

    src/physics/physicsScene.h

    src/filesystem/filesystem.h
    src/filesystem/stream.h
)

add_library(engine STATIC ${engine_src} ${engine_hdr})


