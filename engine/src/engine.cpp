#include "engine.h"

#include "editorController.h"
#include "gr-base/core/file/file.hpp"
#include "imgui.h"
#include "postProcessing.hpp"
#include "shadowMap.hpp"

#include <complex>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

#include "ui/editor.hpp"
#include "ui/project.hpp"
#include "ui/projectMenu.hpp"
#include "ui/ImGuiRender.h"

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/threadpool.hpp>
#include <gr-base/core/event.hpp>
#include <gr-base/core/input.hpp>

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

#include "scriptManager/scriptManager.hpp"
#include "memory.h"

#include "plugin/pluginManager.hpp"

#include <gr-base/ecs/gameObject.hpp>
#include <gr-base/ecs/manager.hpp>

#include <gr-base/core/binary/modelhandle.hpp>

#include <gr-base/core/window.hpp>
#include <gr-base/core/clock.hpp>

#include <filesystem>
#include <gr-ini/config.h>
#include <memory>

Shadowbuffer *m_shadowbuffer = nullptr;

std::unique_ptr<ShadowMap> m_shadowMap = nullptr;

static FunctionTimer mFunctionTimer;

void Engine::Reize(grm::u32 width, grm::u32 height) {
    Screen::SetResolution(width, height);

    iVector2 size((int) width, (int) height);

    Window::SetWindowSize(size);

    // if (ImGuiPostProcess) {
    //     ImGuiPostProcess->resize(size);
    // }
}

void Engine::Initialize() {
    Threadpool::Initialize();

    ImGuiRender::Initialize("#version 330 core");

    AssetsManager::Initialize();

    // Buffer de renderização.
    // ImGuiPostProcess = std::make_shared<PostProcessing>(iVector2{1, 1});

    m_shadowMap = std::make_unique<ShadowMap>(1024, 1024);

    SetupEnable();

    PhysicsScene::Initialize();

    EditorController::Initialize();

    Threadpool::AddTask(&Engine::OnPhysics);
}

void Engine::Shutdown() {
    // if (ImGuiPostProcess) {
        // ImGuiPostProcess = nullptr;
    // }

    ImGuiRender::Shutdown();

    PluginManager::Shutdown();

    PhysicsScene::Shutdown();

    AssetsManager::Shutdown();

    Threadpool::Shutdown();
}

void Engine::Idle()
{
    static auto &frameTime = Statistics::FrameTime[FrameTimeID_MainThread];

    static Time time;

    Clock::deltaTime = time.sleep(60.0f);

    frameTime.setDelta(Clock::deltaTime);
    /*frameTime.setFPS(time.getFPS());*/
    /*Clock::deltaTime = Statistics::m_threadFrame.delta;*/

    // if (m_editor != nullptr)
        // Engine::Update();

    Engine::Render();

    time.end();
}

void Engine::Update() {
    Event::Update();

    mFunctionTimer.start();

    if (Settings::bGameRunning) {
        Manager::Update();
    } else {
        // EditorController::Update(m_editor->getCamera());
    }

    mFunctionTimer.end();

    /*Statistics::m_scriptFrame = {mFunctionTimer.getFps(), mFunctionTimer.getDeltaTime()};*/
}

void Engine::Render() {
    mFunctionTimer.start();
    
    /*
    if (m_editor)
    {
        CameraRenderManager();
    }
    */
    mFunctionTimer.end();

    /*Statistics::m_sceneFrame = {mFunctionTimer.getFps(), mFunctionTimer.getDeltaTime()};*/

    // if (ImGuiPostProcess) {
        // GUIRender();
    // }
}

void Engine::CameraRenderManager()
{
    // shadow
    /*
    {
        m_shadowMap->bind();
        
        grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER);
        grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);
        grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_FRONT);
        grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);
        
        Batching::RenderShadowPass();
    
        m_shadowMap->unbind();
    }
    */


    /*
    auto camera_editor = m_editor->getCamera();
    auto cameras = Camera::GetCameras();

    if (!Settings::bGameRunning && camera_editor != nullptr)
    {
        PreConfigRenderScene(camera_editor);
    } else
    {
        for (auto camera : cameras)
        {
            if (!camera->getGameObject()->getActive() || camera == camera_editor)
            {
                continue;
            }

            PreConfigRenderScene(camera);
        }
    }
    */

    // Captura de posprocessamento para o imgui
    // ImGuiPostProcess->bind();

    // renderização da cena 2d e utilização do pos processamento;
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::red);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_FALSE);
    // grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);

    /*if (!Settings::bGameRunning && camera_editor != nullptr)
    {
        ScreenRender::Render(camera_editor);
    } else
    {
        for (auto camera : cameras)
        {
            if (!camera->getGameObject()->getActive() || camera == camera_editor)
            {
                continue;
            }
    
            ScreenRender::Render(camera);
        }
    }
    */

    // ImGuiPostProcess->unbind();
}

void Engine::PreConfigRenderScene(Camera *camera)
{
    // Seleção de modelos para renderizar
    RenderManager::Process(camera, &(*m_shadowMap));

    camera->rendertarget.bind();

    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);

    RenderScene(camera);

    camera->rendertarget.unbind();
}

void Engine::GRKeyboardDown(grm::u32 key) {
}

void Engine::GRKeyboardUp(grm::u32 key) {}

void Engine::GRCursorPos(float xpos, float ypos) {}

void Engine::GRCursorScrool(float xoffset, float yoffset) {
    if (yoffset == -1.0f) {
        EditorController::movementSpeed += 5.0f;
    } else if (yoffset == 1.0f) {
        EditorController::movementSpeed -= 5.0f;
    }
}

void Engine::DropFile(const char *path) {
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

void Engine::GUIRender() {
    iVector2 scale(Screen::width, Screen::height);
    
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void*)&scale);
    
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    
    ImGuiRender::Begin();
    
    {
        mFunctionTimer.start();

        ModelHandle::Render();

        PluginManager::Render();

        mFunctionTimer.end();
    }
    
    /*Statistics::m_guiFrame = {mFunctionTimer.getFps(), mFunctionTimer.getDeltaTime()};*/
    // m_editor->renderFrame();

    ImGuiRender::End();
}

void Engine::SetupEnable() {
    grr::gRender::SetRenderState(grr::GR_MULTISAMPLE, grr::GR_TRUE);

    grr::gRender::SetRenderState(grr::GR_CULL_FACE, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);

    grr::gRender::SetRenderState(grr::GR_BLEND, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_SRC_ALPHA, grr::GR_ONE_MINUS_SRC_ALPHA);
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

    {
        update();

        render();
    }
}

void Engine::update()
{}

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






