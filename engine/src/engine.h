#pragma once

#include "ui/project.hpp"
#include "ui/viewlayout.hpp"

#include <gr-base/script/default.hpp>

#include <gr-base/ecs/default.hpp>

class Engine {
public:
    enum EngineEvent {
        INITIALIZE     = 1 << 1,
        DESTROY        = 1 << 2,
        RESIZE         = 1 << 3,
        LOOP           = 1 << 7,
        CURSOR_SCROLL  = 1 << 8,
        DROFUNC        = 1 << 9
    };

    static void HandleEvent(EngineEvent event, const void* data);

    static Engine &GetInstance();

    Engine();
    ~Engine();

    void init_menu();

    void init_project(const std::string& path);

    Project &get_project();

    bool has_project() const;

private:
    Project m_project;

    bool bHasProject;

    void init();

    bool check_installed();

    void shutdown();

    void save_engine_config();

    void loop();

    void update();

    void render();

    void render_gui();

    // void create_main_tabbar();

    void drop_file(const std::string &path);

protected:
    void upload(GameObject *gameObject);

    void render_cam();

    static void PreConfigRenderScene(Camera *camera);

    static void RenderScene(const Camera *camera);
};

