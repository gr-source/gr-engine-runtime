#include "gr-engine/managers/viewmanager.hpp"
#include "imgui.h"

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

void ViewManager::Renderer3D()
{
    s_ViewManager->renderer3D();
}

void ViewManager::RendererUI()
{
    s_ViewManager->rendererUI();
}

void ViewManager::RemoveAllView()
{
    s_ViewManager->removeAllView();
}

// private
ViewManager::ViewManager() : s_current(nullptr)
{}

ViewManager::~ViewManager()
{}

void ViewManager::addView(ViewHash hash, std::shared_ptr<ViewLayout> view)
{
    s_viewRegistry.push_back(view.get());

    if (s_current == nullptr)
        s_current = s_viewRegistry.back();

    s_ViewMap.emplace(hash, std::move(view));
}

void ViewManager::addSubView(ViewHash hash, std::shared_ptr<ViewLayout> view)
{
    s_viewRegistryPopup.push_back(view.get());

    s_ViewMap.emplace(hash, std::move(view));
}

ViewLayout* ViewManager::getView(ViewHash hash)
{
    auto it = s_ViewMap.find(hash);
    if (it != s_ViewMap.end())
        return it->second.get();
    return nullptr;
}

void ViewManager::remove(ViewHash hash)
{
    auto it = s_ViewMap.find(hash);
    if (it == s_ViewMap.end())
        return;

    s_viewRegistryPopup.erase(std::remove(s_viewRegistryPopup.begin(), s_viewRegistryPopup.end(), it->second.get()), s_viewRegistryPopup.end());
    s_viewRegistry.erase(std::remove(s_viewRegistry.begin(), s_viewRegistry.end(), it->second.get()), s_viewRegistry.end());

    s_ViewMap.erase(it);
}

void ViewManager::removeAllView()
{
    s_ViewMap.clear();
    s_viewRegistry.clear();

    {
        for (auto& view : s_viewRegistryPopup)
            delete view;

        s_viewRegistryPopup.clear();
    }

    s_current = nullptr;
}

void ViewManager::update()
{
    if (s_current != nullptr)
        s_current->onUpdate();
}

void ViewManager::renderer3D()
{
    if (s_current != nullptr)
        s_current->onRender3D();
}

void ViewManager::rendererUI()
{
    bool bEnableDockSpace = s_viewRegistry.size() > 1;
    if (bEnableDockSpace)
        render_tabbar();

    if (s_current != nullptr)
        s_current->onRenderUI();

    for (auto& view : s_viewRegistryPopup)
    {
        if (ImGui::Begin(view->getName()))
            view->onRenderUI();
        ImGui::End();
    }
}

void ViewManager::render_tabbar()
{
    if (!ImGui::BeginMainMenuBar())
        return;
    
    if (ImGui::BeginTabBar("##render_gui_tab_bar_0"))
    {
        for (auto& view : s_viewRegistry)
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


