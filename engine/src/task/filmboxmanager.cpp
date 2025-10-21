#include "filmboxmanager.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/core/file/file.hpp"


#include "gr-base/core/window.hpp"

#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"
#include "filmboxtask.hpp"

#include "gr-base/ecs/gameObject.hpp"
#include "gr-base/script/transform.hpp"

#include <imgui.h>

// ********** public ********** //
void FilmboxManager::Render()
{
    return GetInstance().render();
}

// ********** private ********** //
FilmboxManager &FilmboxManager::GetInstance()
{
    static FilmboxManager instance;
    return instance;
}

FilmboxManager::FilmboxManager()
{}

FilmboxManager::~FilmboxManager()
{}

std::shared_ptr<TaskInfo> FilmboxManager::load(const std::string &path, std::function<void ()> func)
{
    return TaskManager::Add<FilmboxTask>(path, this);
}

void FilmboxManager::render()
{
    if (resultTask.empty())
    {
        return;
    }

    bool isOpen = true;

    auto mainWindowSize = Window::GetWindowSize() / 2.0f;
    auto beginSize = ImVec2{400, 300};

    ImGui::SetNextWindowSize(beginSize);
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(mainWindowSize.x), static_cast<float>(mainWindowSize.y)} - (beginSize / 2.0f));

    if (ImGui::Begin("FilmboxManager", &isOpen))
    {
        for (auto &gameObject : resultTask)
        {
            auto name = gameObject->getName();

            if (ImGui::CollapsingHeader(name.c_str()))
            {
                auto transform = gameObject->getTransform();

                for (auto &child : transform->getChildren())
                {
                    render_tree_node(child->getGameObject());
                }
            }
        }

        auto avail = ImGui::GetContentRegionAvail();

        if (ImGui::Button("Save", ImVec2(avail.x, 0.0f)))
        {
            auto dir = File::GetAsset("/model");

            for (auto &gameObject : resultTask)
            {
                TaskManager::Add<GameObjectSaveTask>(dir, gameObject, std::bind(&FilmboxManager::GameObjectSave, this, std::placeholders::_1));
            }
        }
    }
    ImGui::End();

    if (!isOpen)
    {
        for (auto &gameObject : resultTask)
        {
            GameObject::Destroy(gameObject);
        }
        resultTask.clear();

        taskResultMap.clear();
    }
}

void FilmboxManager::create_save(const std::string &name, GameObject *root)
{
    if (taskResultMap.find(name) != taskResultMap.end())
    {
        return GameObject::Destroy(root);
    }

    resultTask.emplace_back(root);

    taskResultMap.emplace(name);
}

void FilmboxManager::render_tree_node(const GameObject *gameObject)
{
    auto transform = gameObject->getTransform();

    auto isOpen = ImGui::TreeNodeEx(gameObject->getName().c_str(), !transform->hasChildren() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (!isOpen)
        return;

    for (auto &child : transform->getChildren())
        render_tree_node(child->getGameObject());

    ImGui::TreePop();
}

void FilmboxManager::GameObjectSave(GameObject *go)
{
    resultTask.erase(std::remove(resultTask.begin(), resultTask.end(), go), resultTask.end());

    GameObject::Destroy(go);
}


