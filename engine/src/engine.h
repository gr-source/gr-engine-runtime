#pragma once

#include "ui/project.hpp"
#include "ui/viewlayout.hpp"
#include "ui/editor.hpp"

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

    Engine();
    ~Engine();

    void set_project(const Project &project);

private:
    std::vector<std::unique_ptr<ViewLayout>> v_view_list;

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

protected:
    static void Reize(grm::u32 width, grm::u32 height);

    static void Initialize();

    static void Shutdown();

    static void Idle();

    static void Update();

    static void Render();

    static void CameraRenderManager();

    static void PreConfigRenderScene(Camera *camera);

    static void GRKeyboardDown(grm::u32 key);
    static void GRKeyboardUp(grm::u32 key);
    static void GRCursorPos(float xpos, float ypos);
    static void GRCursorScrool(float xoffset, float yoffset);
    static void DropFile(const char* path);

    static void OnPhysics();

    static void RenderScene(const Camera *camera);
    
    static void GUIRender();
    
    static void SetupEnable();
};

