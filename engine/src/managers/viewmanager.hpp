#pragma once

#include "../ui/viewlayout.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

class ViewManager
{
public:
    static void Initialize();

    static void Shutdown();

    static void Update();

    static void Renderer();

    static void RemoveAllView();

    template <typename T, typename... Args>
    static void Add(Args&&... args);

    static void Remove(ViewLayout* view);

private:
    static ViewManager *s_ViewManager;

    std::unordered_set<ViewLayout*> s_mapRegistry;

    std::vector<ViewLayout*> s_registry;

    ViewLayout* s_current;

    ViewManager();
    ~ViewManager();

    void add(ViewLayout* view);

    void remove(ViewLayout* view);

    void removeAllView();

    void update();

    void renderer();

    void render_tabbar();
};

template <typename T, typename... Args>
inline void ViewManager::Add(Args&&... args)
{
    return s_ViewManager->add(new T(std::forward<Args>(args)...));
}




