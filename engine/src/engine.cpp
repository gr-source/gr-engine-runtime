#include "engine.h"

#include "input.h"
#include "screen.h"
#include "editorController.h"
#include "postProcessing.hpp"

#include "component/meshRender.hpp"
#include "component/camera.hpp"

#include "ui/ImGuiRender.h"

#include "framebuffer/skyboxbuffer.h"
#include "framebuffer/shadowbuffer.h"

#include "resources/resources.h"

#include "physics/physicsScene.h"

#include "filesystem/stream.h"

#include "renderManager.h"
#include "eventManager.h"
#include "screenRender.hpp"
#include "settings.h"
#include "renderer.h"
#include "skybox.h"
#include "Global.h"
#include "timer.h"

#include "scriptManager/scriptManager.hpp"
#include "memory.h"

#include <filesystem>

#include <gr-render/gRender.h>

PostProcessing *Engine::ImGuiPostProcess = nullptr;

PostProcessing *Engine::MainPostProcess = nullptr;

std::thread Engine::m_thread[2];

Skybox* Engine::m_skybox = nullptr;

std::unique_ptr<ScreenRender> pScreenRender = nullptr;

Shadowbuffer* m_shadowbuffer = nullptr;

void Engine::GRWindowResize(u32 width, u32 height) {
    Screen::SetResolution(width, height);

    iVector2 size((int) width, (int) height);

    if (ImGuiPostProcess) {
        PostProcessing::Resize(ImGuiPostProcess, size);
    }
}

void Engine::MainLoop(std::function<void()> func) {
    m_thread[0] = std::thread(OnLoader);
    m_thread[1] = std::thread(OnPhysics);

    Time timer(60.0f);
    while (Settings::IsEngineRunning) {
        Time::deltaTime = gameStatics.updateMs = timer.SleepTick();
        Time::FPS = timer.getFPS();

        if (ScriptManager::SetDeltaTime) {
            ScriptManager::SetDeltaTime(Time::deltaTime);
        }

        Engine::OnUpdate();

        Engine::OnRender();

        func();
    }
}

void Engine::GRKeyboardDown(u32 key) {
    Game::KeyBoardUp(key);
}

void Engine::GRKeyboardUp(u32 key) {}

void Engine::GRCursorPos(float xpos, float ypos) {}

void Engine::GRCursorScrool(float xoffset, float yoffset) {
    if (yoffset == -1.0f) {
        EditorController::movementSpeed += 5.0f;
    } else if (yoffset == 1.0f) {
        EditorController::movementSpeed -= 5.0f;
    }
}

void Engine::GRDropFunc(const char *path) {
    Game::DropFile(path);
}

void Engine::OnInit() {
    Memory::Initialize();

    ImGuiRender::Initialize("#version 330 core");

    // Buffer de renderização.
    MainPostProcess = PostProcessing::Create({1, 1});

    ImGuiPostProcess = PostProcessing::Create({1, 1});

    // Renderizaraçãod da screen paar pos processamento e UI.
    pScreenRender = std::make_unique<ScreenRender>();

    // Shadow provavelmente vou remove-lo
    m_shadowbuffer = &Shadowbuffer::getInstance();

    // Skybox provavelmente vou remove-lo
    m_skybox = &Skybox::getInstance();

    SetupEnable();

    DebugDraw::Initialize();

    ScriptManager::Initialzie();

    ScreenRender::Initialize();

    Game::Initialize();

    RenderManager::Initialize();

    EditorController::Initialize();
}

void Engine::OnUpdate() {
    EditorController::Update();

    if (ScriptManager::Update) {
        ScriptManager::Update();
    }
    if (Entity::Update) {
        Entity::Update();
    }
}

Time tickRender;
void Engine::OnRender() {
    EventManager::ProcessEvent();

    if (ScriptManager::EventUpdate) {
        ScriptManager::EventUpdate();
    }

    tickRender.start();

    if (MainPostProcess) {
        MainFramebuffer();
    }

    gameStatics.renderMs = tickRender.end();

    if (ImGuiPostProcess) {
        UIFramebuffer();
    }
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

void Engine::OnLoader() {
    Resources::Initialize();
    
    Time timer(5.0f);
    
    do {
        timer.SleepTick();
        Resources::OnThread();
    } while (Settings::IsEngineRunning);
}

void Engine::OnDestroy() {
    if (MainPostProcess) {
        PostProcessing::Shutdown(MainPostProcess);
    }

    if (ImGuiPostProcess) {
        PostProcessing::Shutdown(ImGuiPostProcess);
    }
    
    ImGuiRender::Shutdown();
    
    ScriptManager::Shutdown();

    ScreenRender::Shutdown();

    if (m_thread[0].joinable()) {
        m_thread[0].join();
    }
    if (m_thread[1].joinable()) {
        m_thread[1].join();
    }
    
    Memory::Shutdown();
}

void Engine::HandleEvent(EngineEvent event, const void* data) {
    switch (event) {
    case GR_INITIALIZE: {
        OnInit();
        break;
    }
    case GR_RESIZE: {
        const u32* scale = static_cast<const u32*>(data);

        GRWindowResize(scale[0], scale[1]);
        break;
    }
    case GR_KEYBOARD_DOWN: {
        GRKeyboardDown(*static_cast<const u32*>(data));
        break;
    }
    case GR_KEYBOARD_UP: {
        GRKeyboardUp(*static_cast<const u32*>(data));
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
    case GR_MAINLOOP: {
        auto func = *static_cast<const std::function<void()>*>(data);
        if (!func) {
            throw std::runtime_error(Log::red + "Faliure initiaçize function loop." + Log::none);
        }
        MainLoop(func);
        break;
    }
    case GR_DROFUNC: {
        GRDropFunc((const char*)data);
        break;
    }
    case GR_DESTROY: {
        OnDestroy();
        break;
    }
    default:
        break;
    }
}

void Engine::MainFramebuffer() {
    auto camera = Camera::Current();
    if (!camera) {
        return;
    }
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

    // Capitura de posprocessamento para o imgui
    PostProcessing::Bind(ImGuiPostProcess);

    // renderização da cena 2d e utilização do pos processamento;
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::blue);
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
    m_skybox->render();
}

void Engine::UIFramebuffer() {
    iVector2 scale(Screen::width, Screen::height);
    
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void*)&scale);

    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);

    ImGuiRender::Begin();
    Game::RenderUI();
    ImGuiRender::End();
}

void Engine::SetupEnable() {
    grr::gRender::SetRenderState(grr::GR_MULTISAMPLE, grr::GR_TRUE);

    grr::gRender::SetRenderState(grr::GR_CULL_FACE, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);

    grr::gRender::SetRenderState(grr::GR_BLEND, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_SRC_ALPHA, grr::GR_ONE_MINUS_SRC_ALPHA);
}







