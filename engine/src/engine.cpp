#include "engine.h"

#include "editorController.h"

#include "shadowMap.hpp"

#include "ui/editor.hpp"
#include "ui/project.hpp"
#include "ui/projectMenu.hpp"
#include "ui/ImGuiRender.h"

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/threadpool.hpp>
#include <gr-base/core/window.hpp>
#include <gr-base/core/event.hpp>
#include <gr-base/core/clock.hpp>
#include <gr-base/core/input.hpp>

#include <gr-base/ecs/gameObject.hpp>
#include <gr-base/ecs/manager.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

#include "core/statistics.hpp"
#include "core/screen.hpp"
#include "core/timer.hpp"
#include "core/time.hpp"

#include "physics/physicsScene.hpp"

#include "functionTimer.hpp"
#include "screenRender.hpp"
#include "renderManager.h"
#include "settings.h"
#include "renderer.h"
#include "skybox.h"
#include "Global.h"

#include "plugin/pluginManager.hpp"

#include <gr-base/core/binary/modelhandle.hpp>

#include <gr-ini/config.h>

static FunctionTimer mFunctionTimer;

void Engine::PreConfigRenderScene(Camera *camera)
{
    // Seleção de modelos para renderizar
    RenderManager::Process(camera);

    camera->rendertarget.bind();

    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);
    // grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);

    RenderScene(camera);

    camera->rendertarget.unbind();
}

void Engine::OnPhysics() {
    Time time;
    while (Settings::IsEngineRunning()) {
        auto deltaTime = time.sleep(60.0f);

        if (Settings::bGameRunning) {
            PhysicsScene::Update(deltaTime);
        }

        time.end();
    }
}

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
    default:
        break;
    }
}

void Engine::RenderScene(const Camera *camera)
{
    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LESS);
    RenderManager::Renderer();

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    DebugDraw::Renderer(camera);

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    Skybox::Renderer(camera);
}

Engine &Engine::GetInstance()
{
    static Engine engine;
    return engine;
}

// ********** public ********** //
Engine::Engine() : view_layout(nullptr), p_current_project(nullptr)
{}

Engine::~Engine()
{}

void Engine::set_project(const Project &project)
{
    p_current_project = std::make_unique<Project>(project);

    File::SetCurrentProject(project.getPath());

    v_view_list.clear();

    view_layout = &(*v_view_list.emplace_back(std::make_unique<Editor>()));
}

// ********** private ********** //
void Engine::init()
{
    File::SetConfigPath(gfile::get_config_directory());
    
    File::SetCachePath(gfile::get_cache_directory());
    
    File::SetUserPath(gfile::get_data_directory());

    Threadpool::Initialize();

    ImGuiRender::Initialize("#version 330 core");

    if (!check_project_openable())
    {
        view_layout = &(*v_view_list.emplace_back(std::make_unique<ProjectMenu>(this)));
    }
}

bool Engine::check_project_openable()
{
    auto config = config_create();
    if (config == nullptr)
    {
        return false;
    }
    config_load((gfile::get_config_directory() + "/config.ini").c_str(), config);

    {
        auto section = config_find_section(config, "project");
        if (section == nullptr)
        {
            return false;
        }

        set_project(Project(config_get_string(section, "name"), config_get_string(section, "path")));
    }

    config_release(config);

    return true;
}

void Engine::shutdown()
{
    save_current_project();

    ImGuiRender::Shutdown();

    Threadpool::Shutdown();
}

void Engine::save_current_project()
{
    if (p_current_project == nullptr)
    {
        return;
    }

    auto config = config_create();
    if (config == nullptr)
    {
        return;
    }

    {
        auto section = config_create_section(config, "project", nullptr);
        config_set_string(section, "name", p_current_project->getName().c_str());
        config_set_string(section, "path", p_current_project->getPath().c_str());
    }

    config_save((gfile::get_config_directory() + "/config.ini").c_str(), config);

    config_release(config);
}

void Engine::loop()
{
    Event::Update();

    static Time time;

    Clock::deltaTime = time.sleep(60.0f);

    {
        update();

        render();
    }
}

void Engine::update()
{
    if (Settings::bGameRunning)
    {
        Manager::Update();
    } else
    {
        EditorController::Update(Camera::Current());
    }
}

void Engine::render()
{
    render_cam();

    render_gui();
}

void Engine::render_cam()
{
    auto cameras = Camera::GetCameras();
    for (auto camera : cameras)
    {
        PreConfigRenderScene(camera);
    }
}

void Engine::render_gui()
{
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    
    ImGuiRender::Begin();

    if (view_layout != nullptr)
    {
        view_layout->render();
    }

    ImGuiRender::End();
}






