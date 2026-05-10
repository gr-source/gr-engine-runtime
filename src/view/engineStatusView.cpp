#include "gr-engine/view/engineStatusView.hpp"

#include <gr-base/core/scene.hpp>

#include <gr-base/core/thread/threadpool.hpp>

#include <gr-base/managers/gameobjectmanager.hpp>
#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/core/status.hpp>

#include <imgui.h>

void EngineStatusView::onRenderUI()
{
    ImGui::SeparatorText("Scripts");
    ImGui::Text("GameObject: %ld", GameObjectManager::GetCount());

    ImGui::SeparatorText("AssetManager");
    ImGui::Text("Textures: %ld", AssetsManager::GetTextureCount());
    ImGui::Text("Material: %d", AssetsManager::GetMaterialCount());
    ImGui::Text("Mesh: %ld", AssetsManager::GetMeshCount());

    ImGui::SeparatorText("Threads");
    ImGui::Text("Threads: %ld/%ld", Threadpool::GetThreadCount(), Threadpool::GetCount());

    ImGui::SeparatorText("Renderização");

    Scene* scene = Scene::GetCurrent();
    if (scene != nullptr)
    {
        ImGui::Text("Luzes: %ld",       scene->GetLights().size());
        ImGui::Text("Cameras: %ld",     scene->GetCameras().size());
        ImGui::Text("Objects: %ld",     scene->getObjectList().size());
    }

    ImGui::Text("DrawCall Count: %ld", Status::BatchDrawCallCount);
    ImGui::Text("Render Count: %ld", Status::BatchRenderCount);
}

