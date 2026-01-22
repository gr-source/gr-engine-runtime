#include "gr-engine/runtime/editorRuntime.hpp"

EditorRuntime::EditorRuntime()
{}

EditorRuntime::~EditorRuntime()
{
    s_SceneManager = nullptr;
    s_OctreeManager = nullptr;
    s_GameObjectManager = nullptr;
    s_BehaviorStorage = nullptr;
}

void EditorRuntime::initialize()
{
    SceneManager::SetInstance(s_SceneManager.get());
    GameObjectManager::SetInstance(s_GameObjectManager.get());
    BehaviorStorage::SetInstance(s_BehaviorStorage.get());
    OctreeManager::SetInstance(s_OctreeManager.get());
}


