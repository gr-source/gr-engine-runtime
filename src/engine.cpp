#include "gr-engine/engine.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-engine/view/checkupdate.hpp"
#include "gr-engine/view/shadereditor.hpp"

#include "gr-engine/task/filmboxtask.hpp"
#include "gr-engine/ui/project.hpp"
#include "gr-engine/ui/projectMenu.hpp"
#include "gr-engine/ui/ImGuiRender.h"

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/mainthreaddispatcher.hpp>
#include <gr-base/core/window.hpp>
#include <gr-base/core/skybox.hpp>
#include <gr-base/core/clock.hpp>
#include <gr-base/core/input.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

#include <gr-base/core/debugdraw.hpp>

#include "gr-engine/Global.h"

// Managers
#include "gr-engine/managers/viewmanager.hpp"

// Views
#include "gr-engine/view/editor.hpp"

#include <gr-render/gCommon.h>

// ********** public ********** //
Engine::Engine() : bHasProject(false)
{
    logger::info("[Application] Runtime mode 'Engine'.");
}

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
}

void Engine::init_project(const std::string& path)
{
    ViewManager::RemoveAllView();
    if (bHasProject)
    {
        bHasProject = false;

        return ViewManager::AddView<ProjectMenu>();
    }

    bHasProject = m_project.load(path);

    if (!bHasProject)
        return ViewManager::AddView<ProjectMenu>();

    ViewManager::AddView<Editor>();
    ViewManager::AddView<ShaderEditor>();
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
void Engine::initialize()
{
    logger::info("[ Engine ] Initialize '0x%lX'.", this);

    ImGuiRender::Initialize("#version 330 core");

    gr::gRender::SetEnable(GR_CULL_FACE, true);
    gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);

    gr::gRender::SetEnable(GR_BLEND, true);
    gr::gRender::SetRenderState(gr::GR_SRC_ALPHA, gr::GR_ONE_MINUS_SRC_ALPHA);

    ViewManager::AddView<CheckUpdate>(&Engine::init_menu, this);
}

void Engine::shutdown()
{
    save_engine_config();

    ViewManager::RemoveAllView();

    ImGuiRender::Shutdown();

    logger::info("[ Engine ] Shutdown '0x%lX'.", this);
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

void Engine::process()
{
    MainThreadDispatcher::Process();

    update();

    render();
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
    gr::gRender::SetBackgroundColor(Color::blue);
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_COLOR_BUFFER);
    
    ImGuiRender::Begin();

    ViewManager::RendererUI();

    ImGuiRender::End();
}

void Engine::DropFile(const std::string &path)
{
    if (!gfile::exists(path))
    {
        return;
    }

    if (path.find(".fbx") != std::string::npos)
        TaskManager::Add<FilmboxLoadTask>(path);
}



