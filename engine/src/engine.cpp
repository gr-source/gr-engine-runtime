#include "engine.h"

#include "editorController.h"

#include <gr-base/broadphase/bvhtree.hpp>

#include "gr-base/core/scenemanager.hpp"
#include "gr-base/ecs/gameobjectmanager.hpp"
#include "imgui.h"

#include "ui/editor.hpp"
#include "ui/project.hpp"
#include "ui/projectMenu.hpp"
#include "ui/ImGuiRender.h"

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/mainthreaddispatcher.hpp>
#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/rendermanager.hpp>
#include <gr-base/core/window.hpp>
#include <gr-base/core/skybox.hpp>
#include <gr-base/core/clock.hpp>
#include <gr-base/core/input.hpp>

#include <gr-base/ecs/entitymanager.hpp>
#include <gr-base/ecs/gameObject.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

#include <gr-base/core/debugdraw.hpp>

#include "functionTimer.hpp"
#include "settings.h"
#include "Global.h"

#include "ui/shadereditor.hpp"

#include "task/filmboxmanager.hpp"

#include <gr-ini/config.h>
#include <gr-render/gCommon.h>

static FunctionTimer mFunctionTimer;

static Skybox m_skybox;

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
Engine::Engine() : view_layout(nullptr), p_current_project(nullptr)
{}

Engine::~Engine()
{}

void Engine::set_project(const Project &project)
{
    p_current_project = std::make_unique<Project>(project);

    File::SetCurrentProject(project.getPath());

    gfile::AssetStreamPath = project.getPath() + "/assets";

    v_view_list.clear();

    // Editor
    {
        auto &&[name, view] = v_view_list.emplace_back("Editor", std::make_unique<Editor>());

        view_layout = view.get();
    }

    // ShaderEditor
    {
        v_view_list.emplace_back("Shader Editor", std::make_unique<ShaderEditor>(this));
    }

    if (view_layout != nullptr)
    {
        view_layout->onFocus();
    }
}

// ********** private ********** //
void Engine::init()
{
    File::SetConfigPath(gfile::get_config_directory());
    
    File::SetCachePath(gfile::get_cache_directory());
    
    File::SetUserPath(gfile::get_data_directory());

    ImGuiRender::Initialize("#version 330 core");

    EntityManager::Initialize();

    GameObjectManager::Initialize();

    if (!check_project_openable())
    {
        auto &&[name, view] =v_view_list.emplace_back("Project Menu", std::make_unique<ProjectMenu>(this));
        
        view_layout = view.get();
    }

    grr::gRender::SetRenderState(grr::GR_CULL_FACE, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);

    grr::gRender::SetRenderState(grr::GR_BLEND, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_SRC_ALPHA, grr::GR_ONE_MINUS_SRC_ALPHA);

    AssetsManager::LoadTexture("/home/felipy/Downloads/Skyboxes/SunsetSky.png", &Skybox::set_texture, &m_skybox);
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

        auto path = config_get_string(section, "path");
        auto name = config_get_string(section, "name");

        if (gfile::exists(path))
            set_project(Project(name, path));
    }

    config_release(config);

    return p_current_project != nullptr;
}

void Engine::shutdown()
{
    v_view_list.clear();
    view_layout = nullptr;

    save_current_project();

    ImGuiRender::Shutdown();

    GameObjectManager::Shutdown();

    EntityManager::Shutdown();
}

void Engine::save_current_project()
{
    if (p_current_project == nullptr)
        return;

    auto config = config_create();
    if (config == nullptr)
        return;

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
    MainThreadDispatcher::Update();

    {
        update();

        render();
    }
}

void Engine::update()
{
    if (Settings::bGameRunning)
    {
        return EntityManager::Update();
    }

    auto scene = SceneManager::GetCurrent();
    if (scene != nullptr && scene->getCamera())
        EditorController::Update(scene->getCamera());
}

void Engine::render()
{
    render_cam();

    render_gui();
}

void Engine::render_gui()
{
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    
    ImGuiRender::Begin();

    auto dockID = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    create_main_tabbar();

    if (view_layout != nullptr)
    {
        view_layout->onRender();
    }

    FilmboxManager::Render();

    AssetsManager::Render();

    ImGuiRender::End();
}

void Engine::create_main_tabbar()
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }
    
    if (ImGui::BeginTabBar("##render_gui_tab_bar_0"))
    {
        for (auto &&[name, view] : v_view_list)
        {
            if (ImGui::BeginTabItem(name))
            {
                if (view_layout != view.get())
                {
                    if (view_layout != nullptr)
                    {
                        view_layout->onBlur();
                    }

                    view_layout = view.get();
                    view_layout->onFocus();
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::EndMainMenuBar();
}

void Engine::drop_file(const std::string &path)
{
    if (!gfile::exists(path))
    {
        return;
    }

    if (path.find(".fbx") != std::string::npos)
    {
        FilmboxManager::Load(path);
    }
}

// ********** protected ********** //
void Engine::upload(GameObject *gameObject)
{
    if (view_layout == nullptr)
    {
        return;
    }

    view_layout->drop((void *)gameObject);
}

#include <gr-base/core/culling/octreemanager.hpp>

void Engine::render_cam()
{
// #if OCREE_DEBUG
    // OctreeManager::Render();
    bvh::Tree::Render();


// #endif

    if (Settings::bGameRunning)
    {
        for (auto &camera : Camera::GetCameras())
        {
            PreConfigRenderScene(camera);
        }
    } else
    {
        auto scene = SceneManager::GetCurrent();
        if (scene != nullptr && scene->getCamera())
        {
            PreConfigRenderScene(scene->getCamera());
        }
    }

    DebugDraw::PosRender();

    RenderManager::PosRender();
}

void Engine::PreConfigRenderScene(Camera *camera)
{
    RenderManager::PreRender(camera);

    camera->rendertarget.bind();

    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);

    RenderScene(camera);

    camera->rendertarget.unbind();
}

void Engine::RenderScene(const Camera *camera)
{
    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LESS);
    RenderManager::Render();

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    DebugDraw::Render(camera);

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    m_skybox.render(camera);
}


