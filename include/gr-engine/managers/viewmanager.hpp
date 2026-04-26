#pragma once

#include "../ui/viewlayout.hpp"

#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <vector>

class ViewManager
{
public:
    static void Update();

    static void Renderer3D();

    static void RendererUI();

    static void RemoveAllView();

    template <typename T, typename... Args>
    static void AddView(Args&&... args);

    template <typename T, typename... Args>
    static T* AddSubView(Args&&... args);

    template <typename T>
    static T* GetView();

    template <typename T>
    static void RemoveView();

private:
    std::unordered_map<ViewHash, std::shared_ptr<ViewLayout>> s_ViewMap;

    std::vector<ViewLayout*> s_viewRegistryPopup;

    std::vector<ViewLayout*> s_viewRegistry;

    ViewLayout* s_current;

    static ViewManager &GetInstance();

    ViewManager();
    ~ViewManager();

    ViewLayout* addView(ViewHash hash, std::shared_ptr<ViewLayout> view);

    ViewLayout* addSubView(ViewHash hash, std::shared_ptr<ViewLayout> view);

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
    if (GetInstance().GetView<T>() != nullptr)
        return;
    GetInstance().addView(typeid(T).hash_code(), std::make_shared<T>(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
inline T* ViewManager::AddSubView(Args&&... args)
{
    T* view = static_cast<T*>(GetInstance().GetView<T>());
    if (view == nullptr)
        view = static_cast<T*>(GetInstance().addSubView(typeid(T).hash_code(), std::make_shared<T>(std::forward<Args>(args)...)));
    return view;
}

template <typename T>
inline T* ViewManager::GetView()
{
    return static_cast<T*>(GetInstance().getView(typeid(T).hash_code()));
}

template <typename T>
inline void ViewManager::RemoveView()
{
    GetInstance().remove(typeid(T).hash_code());
}


