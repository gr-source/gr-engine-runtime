#pragma once

#include "../ui/viewlayout.hpp"

#include <vector>

class ViewManager
{
public:
    static void Initialize();

    static void Shutdown();

    static void Update();

    static void Renderer3D();

    static void RendererUI();

    static void RemoveAllView();

    template <typename T, typename... Args>
    static void Add(Args&&... args);

    template <typename T, typename... Args>
    static void AddPopup(Args&&... args);

    static void Remove(ViewLayout* view);

private:
    static ViewManager *s_ViewManager;

    std::vector<ViewLayout*> s_viewRegistryPopup;

    std::vector<ViewLayout*> s_viewRegistry;

    ViewLayout* s_current;

    ViewManager();
    ~ViewManager();

    void add(ViewLayout* view);

    void addPopup(ViewLayout* view);

    void remove(ViewLayout* view);

    void removeAllView();

    void update();

    void renderer3D();

    void rendererUI();

    void render_tabbar();
};

template <typename T, typename... Args>
inline void ViewManager::Add(Args&&... args)
{
    return s_ViewManager->add(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
inline void ViewManager::AddPopup(Args&&... args)
{
    return s_ViewManager->addPopup(new T(std::forward<Args>(args)...));
}




