#include "engine.h"

#include "editorController.h"
#include "postProcessing.hpp"
#include "shadowMap.hpp"

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

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

std::shared_ptr<PostProcessing> Engine::ImGuiPostProcess = nullptr;

std::shared_ptr<PostProcessing> Engine::MainPostProcess = nullptr;

Shadowbuffer *m_shadowbuffer = nullptr;

std::unique_ptr<ShadowMap> m_shadowMap = nullptr;

Editor *Engine::m_editor = nullptr; 

static FunctionTimer mFunctionTimer;

void Engine::Reize(grm::u32 width, grm::u32 height) {
    Screen::SetResolution(width, height);

    iVector2 size((int) width, (int) height);

    Window::SetWindowSize(size);

    if (ImGuiPostProcess) {
        ImGuiPostProcess->resize(size);
    }
}

void Engine::Initialize() {
    Threadpool::Initialize();

    ImGuiRender::Initialize("#version 330 core");

    AssetsManager::Initialize();

    // Buffer de renderização.
    MainPostProcess = std::make_shared<PostProcessing>(iVector2{1, 1});
    
    ImGuiPostProcess = std::make_shared<PostProcessing>(iVector2{1, 1});

    m_shadowMap = std::make_unique<ShadowMap>(1024, 1024);

    SetupEnable();

    PhysicsScene::Initialize();

    ProjectMenu::Initialize();

    EditorController::Initialize();

    Threadpool::AddTask(&Engine::OnPhysics);
}

void Engine::Shutdown() {
    if (MainPostProcess) {
        MainPostProcess = nullptr;
    }

    if (ImGuiPostProcess) {
        ImGuiPostProcess = nullptr;
    }

    ImGuiRender::Shutdown();

    ProjectMenu::Shutdown();

    PluginManager::Shutdown();

    PhysicsScene::Shutdown();

    AssetsManager::Shutdown();

    Threadpool::Shutdown();
}

/*
FunctionTimer mMainFunctionTimer;
void Engine::Idle() {
    m_editor = ProjectMenu::GetEditor();

    mMainFunctionTimer.end();
    mMainFunctionTimer.start();
    // auto fps = 60.0f;
    // static Time time;
    Statistics::m_threadFrame = {mMainFunctionTimer.getFps(), mMainFunctionTimer.getDeltaTime()};
    Clock::deltaTime = Statistics::m_threadFrame.delta;

    if (m_editor != nullptr)
        Engine::Update();

    Engine::Render();

    // time.end();
}
*/
void Engine::Idle() {
    m_editor = ProjectMenu::GetEditor();

    auto fps = 60.0f;
    static Time time;
    Statistics::m_threadFrame = {time.getFPS(), (fps > 0.0f ? time.sleep(fps) : time.start())};
    Clock::deltaTime = Statistics::m_threadFrame.delta;

    if (m_editor != nullptr)
        Engine::Update();

    Engine::Render();

    time.end();
}

void Engine::Update() {
    Event::Update();

    mFunctionTimer.start();

    if (Settings::bGameRunning) {
        Manager::Update();
    } else {
        EditorController::Update(m_editor->getCamera());
    }

    mFunctionTimer.end();

    Statistics::m_scriptFrame = {mFunctionTimer.getFps(), mFunctionTimer.getDeltaTime()};
}

void Engine::Render() {
    mFunctionTimer.start();

    if (MainPostProcess && m_editor) {
        CameraRenderManager();
    }
    mFunctionTimer.end();

    Statistics::m_sceneFrame = {mFunctionTimer.getFps(), mFunctionTimer.getDeltaTime()};

    if (ImGuiPostProcess) {
        GUIRender();
    }
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

    // Captura de posprocessamento para o imgui
    ImGuiPostProcess->bind();

    // renderização da cena 2d e utilização do pos processamento;
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::red);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_FALSE);
    // grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);

    if (!Settings::bGameRunning && camera_editor != nullptr)
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

    ImGuiPostProcess->unbind();
}

void Engine::PreConfigRenderScene(const Camera *camera)
{
    // Seleção de modelos para renderizar
    RenderManager::Process(camera, &(*m_shadowMap));

    MainPostProcess->setTextureColor(camera->getColorTexture());

    // Pos processamento
    MainPostProcess->bind();

    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);
    // grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);

    RenderScene(camera);

    MainPostProcess->unbind();
}

void Engine::GRKeyboardDown(grm::u32 key) {
    auto editor = ProjectMenu::GetEditor();
    if (editor)
        editor->KeyBoardUp(key);
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
    auto editor = ProjectMenu::GetEditor();
    if (editor)
        editor->DropFile(path);
}

void Engine::OnPhysics() {
    /*
    Time time;
    while (Settings::IsEngineRunning()) {
        auto deltaTime = time.getDeltaTime();
        if (Settings::bGameRunning) {
            PhysicsScene::Update(deltaTime);
        }
        time.end();
    }
    */
}

void Engine::HandleEvent(EngineEvent event, const void* data) {
    switch (event) {
    case GR_INITIALIZE: {
        Initialize();
        break;
    }
    case GR_RESIZE: {
        const grm::u32* scale = static_cast<const grm::u32*>(data);

        Reize(scale[0], scale[1]);
        break;
    }
    case GR_KEYBOARD_DOWN: {
        GRKeyboardDown(*static_cast<const grm::u32*>(data));
        break;
    }
    case GR_KEYBOARD_UP: {
        GRKeyboardUp(*static_cast<const grm::u32*>(data));
        break;
    }
    case GR_CURSOR_POS: {
        auto pos = static_cast<const float*>(data);
        GRCursorPos(pos[0], pos[1]);
        break;
    }
    case GR_CURSOR_SCROLL: {
        auto offset = static_cast<const float*>(data);
        GRCursorScrool(offset[0], offset[1]);
        break;
    }
    case GR_IDLE: {
        Idle();
        break;
    }
    case GR_DROFUNC: {
        DropFile(reinterpret_cast<const char *>(data));
        break;
    }
    case GR_DESTROY: {
        Shutdown();
        break;
    }
    default:
        break;
    }
}

PostProcessing *Engine::GetImGuiPostprocess() {
    return ImGuiPostProcess != nullptr ? &(*ImGuiPostProcess) : nullptr;
}

PostProcessing *Engine::GetMainPostprocess() {
    return MainPostProcess != nullptr ? &(*MainPostProcess) : nullptr;
}

void Engine::RenderScene(const Camera *camera) {
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

        ProjectMenu::Render();

        ModelHandle::Render();

        PluginManager::Render();

        mFunctionTimer.end();
    }
    
    Statistics::m_guiFrame = {mFunctionTimer.getFps(), mFunctionTimer.getDeltaTime()};
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







