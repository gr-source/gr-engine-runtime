#include "engine.h"

#include "editorController.h"

#include <cstddef>
#include <gr-base/broadphase/bvhtree.hpp>

#include "gr-base/core/console.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include "imgui.h"

#include "task/filmboxtask.hpp"
#include "ui/project.hpp"
#include "ui/projectMenu.hpp"
#include "ui/ImGuiRender.h"

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/mainthreaddispatcher.hpp>
#include <gr-base/core/window.hpp>
#include <gr-base/core/skybox.hpp>
#include <gr-base/core/clock.hpp>
#include <gr-base/core/input.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

#include <gr-base/core/debugdraw.hpp>

#include "functionTimer.hpp"
#include "Global.h"

#include "ui/shadereditor.hpp"

// Managers
#include "managers/viewmanager.hpp"

// Views
#include "view/installer.hpp"
#include "view/editor.hpp"

#include <gr-render/gCommon.h>

static FunctionTimer mFunctionTimer;

// static Skybox m_skybox;

void Engine::HandleEvent(EngineEvent event, const void* data)
{
    auto &engine = GetInstance();
    switch (event) {
    case INITIALIZE:
        return engine.init();
    case LOOP:
        return engine.loop();
    case DESTROY:
        return engine.shutdown();
    case DROFUNC:
        return engine.drop_file(static_cast<const char *>(data));
    default:
        break;
    }
}

Engine &Engine::GetInstance()
{
    static Engine engine;
    return engine;
}

// ********** public ********** //
Engine::Engine() : bHasProject(false)
{}

Engine::~Engine()
{}

void Engine::init_menu()
{
    std::string project_path;

    gConfig* config = config_create();
    if (config != NULL)
    {
        std::string path = gfile::get_config_directory() + "/config.ini";

        if (config_load(config, path.c_str()))
        {
            gSection* section = config_find_section(config, "project");
            if (section != NULL)
            {
                const char* path = config_get_string(section, "path");
                if (path != NULL && gfile::exists(path))
                    project_path = path;
            }
        }
        config_release(config);
    }

    init_project(project_path);

    /*
    if (!check_project_openable())
        
    // AssetsManager::LoadTexture("/home/NoirVoid/Downloads/sky_17_2k/sky_17_2k.png", &Skybox::set_texture, &m_skybox);
    */
}

void Engine::init_project(const std::string& path)
{
    ViewManager::RemoveAllView();
    if (bHasProject)
    {
        bHasProject = false;

        return ViewManager::Add<ProjectMenu>();
    }

    bHasProject = m_project.load(path);

    if (!bHasProject)
        return ViewManager::Add<ProjectMenu>();

    ViewManager::Add<Editor>();
}

Project& Engine::get_project()
{
    return m_project;
}

bool Engine::has_project() const
{
    return bHasProject;
}

// ********** private ********** //
void Engine::init()
{
    File::SetConfigPath(gfile::get_config_directory());
    
    File::SetCachePath(gfile::get_cache_directory());
    
    File::SetUserPath(gfile::get_data_directory());

    ImGuiRender::Initialize("#version 330 core");

    ViewManager::Initialize();

    grr::gRender::SetEnable(GR_CULL_FACE, true);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);

    grr::gRender::SetEnable(GR_BLEND, true);
    grr::gRender::SetRenderState(grr::GR_SRC_ALPHA, grr::GR_ONE_MINUS_SRC_ALPHA);

    if (!check_installed())
        return ViewManager::Add<Installer>();

    init_menu();
}

bool Engine::check_installed()
{
    return gfile::exists(gfile::get_home_directory() + "/.config/gr-engine");
}

void Engine::shutdown()
{
    save_engine_config();

    ImGuiRender::Shutdown();

    ViewManager::Shutdown();
}

void Engine::save_engine_config()
{
    auto config = config_create();
    if (config == NULL)
        return;

    if (has_project())
    {
        auto section = config_create_section(config, "project");
        config_set_string(section, "path", (m_project.path + "/metadata.ini").c_str());
    }

    config_save(config, (gfile::get_config_directory() + "/config.ini").c_str());

    config_release(config);
}

void Engine::loop()
{
    MainThreadDispatcher::Update();

    {
        update();

        render();
    }
}

void Engine::update()
{
    ViewManager::Update();
}

void Engine::render()
{
    ViewManager::Renderer3D();

    render_gui();
}

void Engine::render_gui()
{
    grr::gRender::SetBackgroundColor(Color::blue);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    
    ImGuiRender::Begin();

    ViewManager::RendererUI();

    ImGuiRender::End();
}

void Engine::drop_file(const std::string &path)
{
    if (!gfile::exists(path))
    {
        return;
    }

    if (path.find(".fbx") != std::string::npos)
        TaskManager::Add<GameObject*, FilmboxLoadTask>(path);
}

// ********** protected ********** //
void Engine::upload(GameObject *gameObject)
{
    /*
    if (view_layout == nullptr)
        return;

    view_layout->drop((void *)gameObject);
    */
}


