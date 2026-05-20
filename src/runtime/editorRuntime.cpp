#include "gr-engine/runtime/editorRuntime.hpp"

EditorRuntime::EditorRuntime()
{}

EditorRuntime::~EditorRuntime()
{
    s_BVHTreeManager = nullptr;
    s_OctreeManager = nullptr;
}

void EditorRuntime::initialize()
{
    OctreeManager::SetInstance(s_OctreeManager.get());
    BVHTreeManager::SetInstance(s_BVHTreeManager.get());
}


