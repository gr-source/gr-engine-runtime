#include "view/popup/filmboximport.hpp"

#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"
#include "gr-base/core/task/runtime/meshSaveTask.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "managers/viewmanager.hpp"

#include <gr-base/script/transform.hpp>

#include <gr-base/core/gameObject.hpp>

#include "engine.h"

#include <imgui.h>

FilmboxImport::FilmboxImport(GameObjectID gameObjectID, std::vector<MeshID> meshes) : m_gameObjectID(gameObjectID), m_meshes(meshes)
{}

void FilmboxImport::onRenderUI()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    auto beginSize = ImVec2{400, 300};

    ImGui::SetNextWindowSize(beginSize);
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(center.x), static_cast<float>(center.y)} - (beginSize / 2.0f));
    ImGui::Begin("##filmboximport_begin", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SeparatorText("Filmbox Import");

    // for (auto &gameObject : resultTask)
    {
        auto gameObject = GameObject::Get(m_gameObjectID);
        auto name = gameObject->getName();

        if (ImGui::CollapsingHeader(name.c_str()))
        {
            auto transform = gameObject->getTransform();

            for (auto &child : transform->getChildren())
            {
                render_tree_node(child->GetGameObject());
            }
        }
    }

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("Save", ImVec2(avail.x, 0.0f)))
    {
        Engine& engine = Engine::GetInstance();

        Project& project = engine.get_project();
        
        TaskManager::Add<int, GameObjectSaveTask>(
                project.path + "/assets/model",
                m_gameObjectID,
                std::bind(&FilmboxImport::onSavedGameObject, this, std::placeholders::_1)
            );

        for (auto& meshID : m_meshes)
            TaskManager::Add<MeshID, MeshSaveTask>(project.path + "/assets/mesh", meshID, nullptr);
    }

    ImGui::End();
}

void FilmboxImport::render_tree_node(const GameObject *gameObject)
{
    auto transform = gameObject->getTransform();

    auto isOpen = ImGui::TreeNodeEx(gameObject->getName().c_str(), !transform->hasChildren() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (!isOpen)
        return;

    for (auto &child : transform->getChildren())
        render_tree_node(child->GetGameObject());

    ImGui::TreePop();
}

void FilmboxImport::onSavedGameObject(GameObjectID gameObject)
{
    GameObject::DestroyImediate(GameObject::Get(gameObject));

    ViewManager::Remove(this);
}


