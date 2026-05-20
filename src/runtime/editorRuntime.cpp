#include "gr-engine/runtime/editorRuntime.hpp"

EditorRuntime::EditorRuntime()
{}

EditorRuntime::~EditorRuntime()
{
    s_SceneManager = nullptr;
    s_BVHTreeManager = nullptr;
    s_OctreeManager = nullptr;
    s_RenderSystem = nullptr;
}

void EditorRuntime::initialize()
{
    SceneManager::SetInstance(s_SceneManager.get());
    OctreeManager::SetInstance(s_OctreeManager.get());
    BVHTreeManager::SetInstance(s_BVHTreeManager.get());
    RenderSystem::SetInstance(s_RenderSystem.get());
}


