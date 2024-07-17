project("engine")

set(guizmo_src
    src/guizmo/ImGuizmo.cpp
)

set(guizmo_hdr
    src/guizmo/ImGuizmo.h
)

set(ecs_src
    src/ecs/entity.cpp
    src/ecs/component.cpp
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

set(simplexNoise_src
    src/simplexNoise/SimplexNoise.cpp
)

set(simplexNoise_hdr
    src/simplexNoise/SimplexNoise.h
)

set(editor_src
    src/editor/editorController.cpp
)

set(editor_hdr
    src/editor/editorController.hpp
)

set(engine_src
    ${guizmo_src}
    ${octree_src}
    ${editor_src}
    ${simplexNoise_src}

    src/gl.cpp
    src/log.cpp
    src/Game.cpp
    src/timer.cpp
    src/input.cpp
    src/buffer.cpp
    src/engine.cpp
    src/memory.cpp
    src/Global.cpp
    src/screen.cpp
    src/render.cpp
    src/random.cpp
    src/settings.cpp
    src/gameObject.cpp
    src/taskRuntime.cpp
    src/screenRender.cpp
    src/eventManager.cpp
    src/renderManager.cpp
    src/objectManager.cpp
    src/bufferManager.cpp

    src/component/camera.cpp
    src/component/terrain.cpp
    src/component/renderer.cpp
    src/component/animator.cpp
    src/component/collider.cpp
    src/component/Transform.cpp
    src/component/Rigidbody.cpp
    src/component/MeshRender.cpp
    src/component/meshFilter.cpp
    src/component/boxCollider.cpp
    src/component/meshCollider.cpp
    src/component/chunkManager.cpp
    src/component/voxelSettings.cpp
    src/component/lightComponent.cpp
    src/component/chunkController.cpp
    src/component/cameraComponent.cpp
    src/component/playerController.cpp
    src/component/cameraController.cpp
    src/component/characterController.cpp
    
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
    
    src/scene/Scene.cpp
    src/scene/SceneManager.cpp
    
    ${ecs_src}

    src/ui/View.cpp
    src/ui/ImGuiRender.cpp

    src/culling/occlusion.cpp
    src/culling/frustum.cpp

    src/filesystem/filesystem.cpp
    src/filesystem/stream.cpp

    src/physics/physicsScene.cpp

    src/framebuffer/framebuffer.cpp
)

set(engine_hdr
    ${guizmo_hdr}
    ${octree_hdr}
    ${editor_hdr}
    ${simplexNoise_hdr}
    
    src/log.h
    src/Game.h
    src/timer.h
    src/input.h
    src/buffer.h
    src/engine.h
    src/memory.h
    src/Global.h
    src/screen.h
    src/render.h
    src/random.hpp
    src/settings.h
    src/gameObject.h
    src/taskRuntime.h
    src/screenRender.h
    src/eventManager.h
    src/renderManager.h
    src/objectManager.h
    src/bufferManager.h

    src/component/camera.h
    src/component/renderer.h
    src/component/animator.h
    src/component/collider.h
    src/component/Transform.h
    src/component/terrain.hpp
    src/component/Rigidbody.h
    src/component/MeshRender.h
    src/component/meshFilter.h
    src/component/boxCollider.h
    src/component/meshCollider.h
    src/component/chunkManager.hpp
    src/component/lightComponent.h
    src/component/voxelSettings.hpp
    src/component/chunkController.h
    src/component/cameraComponent.h
    src/component/playerController.h
    src/component/cameraController.h
    src/component/characterController.h

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

    src/scene/Scene.h
    src/scene/SceneManager.h

    ${ecs_hdr}

    src/ui/View.h
    src/ui/ImGuiRender.h

    src/culling/occlusion.h
    src/culling/frustum.h
    
    src/filesystem/filesystem.h
    src/filesystem/stream.h

    src/physics/physicsScene.h

    src/framebuffer/framebuffer.hpp
)

add_library(engine STATIC ${engine_src} ${engine_hdr})


