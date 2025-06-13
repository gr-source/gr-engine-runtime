#pragma once

#include "ui/project.hpp"
#include "ui/viewlayout.hpp"
#include "ui/editor.hpp"

#include <functional>
#include <memory>
#include <vector>

class Engine {
public:
    enum EngineEvent {
        INITIALIZE     = 1 << 1,
        DESTROY        = 1 << 2,
        RESIZE         = 1 << 3,
        KEYBOARD_DOWN  = 1 << 4,
        KEYBOARD_UP    = 1 << 5,
        CURSOR_POS     = 1 << 6,
        LOOP           = 1 << 7,
        CURSOR_SCROLL  = 1 << 8,
        DROFUNC        = 1 << 9
    };

    static void HandleEvent(EngineEvent event, const void* data);

    static Engine &GetInstance();

    std::function<void ()> f_function_render;

    Engine();
    ~Engine();

    void set_project(const Project &project);

private:
    std::vector<std::tuple<const char *, std::unique_ptr<ViewLayout>>> v_view_list;

    std::unique_ptr<Project> p_current_project;

    ViewLayout *view_layout;

    void init();

    bool check_project_openable();

    void shutdown();

    void save_current_project();

    void loop();

    void update();

    void render();

    void render_cam();

    void render_gui();

    void create_main_tabbar();

    void drop_file(const std::string &path);

protected:
    void upload(GameObject *gameObject);

    void default_render();

    static void PreConfigRenderScene(Camera *camera);

    static void RenderScene(const Camera *camera);
};

