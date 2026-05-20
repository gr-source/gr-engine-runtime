#include "gr-engine/view/spawnObject.hpp"

#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/script/transform.hpp>

#include "gr-engine/engine.hpp"
#include "imgui.h"

SpawnObject::SpawnObject() : numInstances(100), position({0.0f, 0.0f})
{}

void SpawnObject::onRenderUI()
{
    ImGui::InputInt("Num", &numInstances);

    if (ImGui::Button("Spawn"))
    {
        AssetsManager::LoadGameObject("Cubo", &SpawnObject::SetPrefab, this);
    }

    ImGui::SameLine();

    if (ImGui::Button("Despawn"))
    {
        for (auto& handle : m_instances)
            // GameObject::DestroyImediate(handle);

        m_instances.clear();
    }

}

void SpawnObject::SetPrefab(Entity prefabID)
{
    int distance = numInstances / 2;

    int currentX = floor(position.x / 16.0f);
    int currentZ = floor(position.y / 16.0f);

    /*
    for (int x=-distance;x<distance;x++)
    {
        for (int z=-distance;z<distance;z++)
        {
            int targetX = (currentX + x);
            int targetZ = (currentZ + z);

            auto gameobjectID = GameObject::Instantiate(prefabID);

            auto gameObject = GameObject::Get(gameobjectID);
            gameObject->SetName("Test[" + std::to_string(targetX) + "][" + std::to_string(targetZ) + "]");

            auto transform = gameObject->GetTransform();
            transform->setLocalPosition({(float)targetX * 5.5f, 0.0f, (float)targetZ * 4.5f});

            m_instances.push_back(gameobjectID);
        }
    }
    */

    // position += Vector2{(float)distance * 16.0f * 2.0f, 0.0f};

    // GameObject::DestroyImediate(prefabID);

    // MainThreadDispatcher::AddDelayed(10.0f, &TestPopup::Spawn, this, prefabID);
}


