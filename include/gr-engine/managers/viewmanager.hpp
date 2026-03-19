#pragma once

#include "../ui/viewlayout.hpp"

#include <memory>
#include <typeinfo>
#include <unordered_map>
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
    static void AddView(Args&&... args);

    template <typename T, typename... Args>
    static void AddSubView(Args&&... args);

    template <typename T>
    static ViewLayout* GetView();

    template <typename T>
    static void RemoveView();

private:
    static ViewManager *s_ViewManager;

    std::unordered_map<ViewHash, std::shared_ptr<ViewLayout>> s_ViewMap;

    std::vector<ViewLayout*> s_viewRegistryPopup;

    std::vector<ViewLayout*> s_viewRegistry;

    ViewLayout* s_current;

    ViewManager();
    ~ViewManager();

    void addView(ViewHash hash, std::shared_ptr<ViewLayout> view);

    void addSubView(ViewHash hash, std::shared_ptr<ViewLayout> view);

    ViewLayout* getView(ViewHash hash);

    void remove(ViewHash hash);

    void removeAllView();

    void update();

    void renderer3D();

    void rendererUI();

    void render_tabbar();
};

template <typename T, typename... Args>
inline void ViewManager::AddView(Args&&... args)
{
    if (s_ViewManager->GetView<T>() != nullptr)
        return;
    s_ViewManager->addView(typeid(T).hash_code(), std::make_shared<T>(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
inline void ViewManager::AddSubView(Args&&... args)
{
    if (s_ViewManager->GetView<T>() != nullptr)
        return;
    s_ViewManager->addSubView(typeid(T).hash_code(), std::make_shared<T>(std::forward<Args>(args)...));
}

template <typename T>
inline ViewLayout* ViewManager::GetView()
{
    return s_ViewManager->getView(typeid(T).hash_code());
}

template <typename T>
inline void ViewManager::RemoveView()
{
    s_ViewManager->remove(typeid(T).hash_code());
}


