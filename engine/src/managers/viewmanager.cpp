#include "viewmanager.hpp"

#include <cstddef>
#include <imgui.h>

#include <algorithm>

ViewManager* ViewManager::s_ViewManager = nullptr;

void ViewManager::Initialize()
{
    if (s_ViewManager == nullptr)
        s_ViewManager = new ViewManager();
}

void ViewManager::Shutdown()
{
    if (s_ViewManager != nullptr)
        delete s_ViewManager;
}

void ViewManager::Update()
{
    s_ViewManager->update();
}

void ViewManager::Renderer()
{
    s_ViewManager->renderer();
}

void ViewManager::RemoveAllView()
{
    s_ViewManager->removeAllView();
}

void ViewManager::Remove(ViewLayout *view)
{
    s_ViewManager->remove(view);
}

// private
ViewManager::ViewManager() : s_current(nullptr)
{}

ViewManager::~ViewManager()
{
    auto temp = s_registry;
    for (auto& view : temp)
        remove(view);
}

void ViewManager::add(ViewLayout* view)
{
    s_registry.push_back(view);
    s_mapRegistry.emplace(view);

    if (s_current == nullptr)
        s_current = s_registry.back();
}

void ViewManager::remove(ViewLayout* view)
{
    if (s_mapRegistry.find(view) == s_mapRegistry.end())
        return;

    s_mapRegistry.erase(view);
    s_registry.erase(std::remove(s_registry.begin(), s_registry.end(), view), s_registry.end());

    if (s_current == view)
        s_current = nullptr;

    delete view;
}

void ViewManager::removeAllView()
{
    auto temp = s_registry;
    for (auto& view : temp)
        delete view;

    s_mapRegistry.clear();
    s_registry.clear();

    s_current = nullptr;
}

void ViewManager::update()
{
    if (s_current != nullptr)
        s_current->onUpdate();
}

void ViewManager::renderer()
{
    bool bEnableDockSpace = s_registry.size() > 1;
    if (bEnableDockSpace)
    {
        render_tabbar();
    }

    if (s_current != nullptr)
        s_current->onRender();
}

void ViewManager::render_tabbar()
{
    if (!ImGui::BeginMainMenuBar())
        return;
    
    if (ImGui::BeginTabBar("##render_gui_tab_bar_0"))
    {
        for (auto& view : s_registry)
        {
            if (ImGui::BeginTabItem(view->getName()))
            {
                if (s_current != view)
                {
                    if (s_current != nullptr)
                        s_current->onBlur();

                    s_current = view;
                    s_current->onFocus();
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::EndMainMenuBar();
}


