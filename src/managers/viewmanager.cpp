#include "gr-engine/managers/viewmanager.hpp"

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

void ViewManager::Remove(ViewLayout *view)
{
    s_ViewManager->remove(view);
}

// private
ViewManager::ViewManager() : s_current(nullptr)
{}

ViewManager::~ViewManager()
{
    auto temp = s_viewRegistry;
    for (auto& view : temp)
        remove(view);
}

void ViewManager::add(ViewLayout* view)
{
    s_viewRegistry.push_back(view);

    if (s_current == nullptr)
        s_current = s_viewRegistry.back();
}

void ViewManager::addPopup(ViewLayout* view)
{
    s_viewRegistryPopup.push_back(view);
}

void ViewManager::remove(ViewLayout* view)
{
    s_viewRegistryPopup.erase(std::remove(s_viewRegistryPopup.begin(), s_viewRegistryPopup.end(), view), s_viewRegistryPopup.end());
    s_viewRegistry.erase(std::remove(s_viewRegistry.begin(), s_viewRegistry.end(), view), s_viewRegistry.end());

    if (s_current == view)
        s_current = nullptr;

    delete view;
}

void ViewManager::removeAllView()
{
    {
        auto temp = s_viewRegistry;
        for (auto& view : temp)
            delete view;

        s_viewRegistry.clear();
    }

    {
        auto temp = s_viewRegistryPopup;
        for (auto& view : temp)
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
        view->onRenderUI();
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


