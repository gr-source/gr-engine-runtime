#include "engine.h"

#include "editorController.h"
#include "postProcessing.hpp"
#include "screen.h"
#include "input.h"

#include "script/meshRender.hpp"
#include "script/camera.hpp"

#include "ui/projectMenu.hpp"
#include "ui/ImGuiRender.h"

#include "core/assetsManager.hpp"
#include "core/threadpool.hpp"
#include "core/statistics.hpp"
#include "core/event.hpp"
#include "core/time.hpp"

#include "physics/physicsScene.h"

#include "renderManager.h"
#include "screenRender.hpp"
#include "settings.h"
#include "renderer.h"
#include "skybox.h"
#include "Global.h"
#include "timer.h"

#include "scriptManager/scriptManager.hpp"
#include "memory.h"

#include "ecs/gameObject.h"
#include "ecs/manager.hpp"

#include <filesystem>

PostProcessing *Engine::ImGuiPostProcess = nullptr;

PostProcessing *Engine::MainPostProcess = nullptr;

Shadowbuffer* m_shadowbuffer = nullptr;

void Engine::Reize(grm::u32 width, grm::u32 height) {
    Screen::SetResolution(width, height);

    iVector2 size((int) width, (int) height);

    if (ImGuiPostProcess) {
        PostProcessing::Resize(ImGuiPostProcess, size);
    }
}

void Engine::Initialize() {
    Memory::Initialize();

    ThreadPool::Initialize();

    ImGuiRender::Initialize("#version 330 core");

    AssetsManager::Initialize();

    // Buffer de renderização.
    MainPostProcess = PostProcessing::Create({1, 1});

    ImGuiPostProcess = PostProcessing::Create({1, 1});

    // Shadow provavelmente vou remove-lo
    // m_shadowbuffer = &Shadowbuffer::getInstance();

    SetupEnable();

    Input::Initialize();

    ProjectMenu::Initialize();

    EditorController::Initialize();
}

void Engine::Idle() {
    auto fps = Editor::GetFps();
    static Time time;
    Statistics::m_threadFrame = {time.getFPS(), (fps > 0.0f ? time.sleep(fps) : time.start())};
    TimeLast::deltaTime = Statistics::m_threadFrame.delta;

    if (ScriptManager::SetDeltaTime) {
        ScriptManager::SetDeltaTime(TimeLast::deltaTime);
    }

    Engine::Update();

    Engine::Render();

    time.end();
}

void Engine::Update() {
    Time time;
    time.end();

    EditorController::Update();

    Manager::Update();

    Statistics::m_scriptFrame = {time.getFPS(), time.start()};
}

void Engine::Render() {
    Event::ProcessEvent();

    Time time;
    time.end();

    if (MainPostProcess) {
        MainFramebuffer();
    }

    Statistics::m_sceneFrame = {time.getFPS(), time.start()};

    time.end();
    if (ImGuiPostProcess) {
        UIFramebuffer();
    }
    Statistics::m_guiFrame = {time.getFPS(), time.start()};
}

void Engine::Shutdown() {
    if (MainPostProcess) {
        PostProcessing::Shutdown(MainPostProcess);
    }

    if (ImGuiPostProcess) {
        PostProcessing::Shutdown(ImGuiPostProcess);
    }

    ImGuiRender::Shutdown();

    ProjectMenu::Shutdown();

    GameObject::Clear();

    AssetsManager::Shutdown();

    ThreadPool::Shutdown();
    
    Memory::Shutdown();
}

void Engine::GRKeyboardDown(grm::u32 key) {
    Editor::KeyBoardUp(key);
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

void Engine::GRDropFunc(const char *path) {
    Editor::DropFile(path);
}

void Engine::OnPhysics() {
    /*
    PhysicsScene&  pPhysicsScene  = PhysicsScene::getInstance();
    Time timer(60.0f);

    do {
        gameStatics.physicsMs = timer.SleepTick();

        if (Settings::IsGameRunning)
            pPhysicsScene.Simulate(gameStatics.physicsMs);
    } while (Settings::IsEngineRunning);
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
        GRDropFunc((const char*)data);
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

void Engine::MainFramebuffer() {
    // Seleção de modelos para renderizar
    RenderManager::Process();

    // Pos processamento
    PostProcessing::Bind(MainPostProcess);

    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_TRUE);

    RenderScene();

    // Fim do posprocessamento
    PostProcessing::Unbind();

    // Captura de posprocessamento para o imgui
    PostProcessing::Bind(ImGuiPostProcess);

    // renderização da cena 2d e utilização do pos processamento;
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::red);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_FALSE);
    grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);

    ScreenRender::Render(PostProcessing::GetTextureColor(MainPostProcess));

    PostProcessing::Unbind();
}

void Engine::RenderScene() {
    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LESS);
    RenderManager::Renderer();

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_ALWAYS);
    DebugDraw::Renderer();

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    Skybox::Renderer();
}

void Engine::UIFramebuffer() {
    iVector2 scale(Screen::width, Screen::height);
    
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void*)&scale);

    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);

    ImGuiRender::Begin();
    ProjectMenu::Render();

    ImGuiRender::End();
}

void Engine::SetupEnable() {
    grr::gRender::SetRenderState(grr::GR_MULTISAMPLE, grr::GR_TRUE);

    grr::gRender::SetRenderState(grr::GR_CULL_FACE, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);

    grr::gRender::SetRenderState(grr::GR_BLEND, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_SRC_ALPHA, grr::GR_ONE_MINUS_SRC_ALPHA);
}







