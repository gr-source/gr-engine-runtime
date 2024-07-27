#include "engine.h"
#include "input.h"
#include "screen.h"
#include "editorController.h"

#include "component/meshRender.hpp"
#include "component/camera.hpp"

#include "ui/ImGuiRender.h"

#include "framebuffer/screenbuffer.h"
#include "framebuffer/skyboxbuffer.h"
#include "framebuffer/shadowbuffer.h"
#include "framebuffer/imguibuffer.h"
#include "framebuffer/gBuffer.h"

#include "resources/resources.h"

#include "physics/physicsScene.h"

#include "filesystem/stream.h"

#include "renderManager.h"
#include "eventManager.h"
#include "screenRender.h"
#include "settings.h"
#include "renderer.h"
#include "skybox.h"
#include "Global.h"
#include "timer.h"

#include "scriptManager/scriptManager.hpp"
#include "memory.h"

#include <linux/input-event-codes.h>
#include <filesystem>

#include <gr-render/gRender.h>

Imguibuffer* pImguibuffer = nullptr;

ScreenBuffer* pScreenBuffer = nullptr;

void Engine::GRWindowResize(u32 width, u32 height) {
    Screen::SetResolution(width, height);

    Vector2 size(static_cast<float>(width), static_cast<float>(height));
    
    if (pImguibuffer)
        pImguibuffer->resize(size);
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
    switch (key) {
    case KEY_ESC: {
        ImGuiIO& io = ImGui::GetIO();

        io.MouseDrawCursor = !io.MouseDrawCursor;
        break;
    }
    default:
        break;
    }
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
}

std::unique_ptr<Game> Engine::m_game = nullptr;

Skybox* Engine::m_skybox = nullptr;

std::unique_ptr<ScreenRender> pScreenRender = nullptr;

std::unique_ptr<ImGuiRender> pImGuiRender = nullptr;

Shadowbuffer* m_shadowbuffer = nullptr;

GBuffer* m_gbuffer = nullptr;

std::thread Engine::m_thread[2];

void Engine::OnInit() {
    Settings::IsEngineRunning = true;

    Memory::Initialize();

    pImGuiRender = std::make_unique<ImGuiRender>("#version 330 core");

    pScreenBuffer = &ScreenBuffer::getInstance();
    pImguibuffer = &Imguibuffer::getInstance();

    pScreenRender = std::make_unique<ScreenRender>();

    m_shadowbuffer = &Shadowbuffer::getInstance();

    m_skybox = &Skybox::getInstance();

    m_game = std::make_unique<Game>();

    SetupEnable();

    DebugDraw::initialize();

    ScriptManager::Initialzie();

    RenderManager::Initialize();

    EditorController::Initialize();
}

void Engine::OnUpdate() {
    Entity::Update();

    EditorController::Update();
}

void Engine::OnRender() {
    EventManager::ProcessEvent();

    MainFramebuffer();

    #if !IS_GAME
    UIFramebuffer();
    #endif
}

void Engine::OnPhysics() {
    PhysicsScene&  pPhysicsScene  = PhysicsScene::getInstance();
    Time timer(60.0f);

    do {
        gameStatics.physicsMs = timer.SleepTick();

        if (Settings::IsGameRunning)
            pPhysicsScene.Simulate(gameStatics.physicsMs);
    } while (Settings::IsEngineRunning);
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
    ScriptManager::Shutdown();

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

Time tickRender;
void Engine::MainFramebuffer() {
    gameStatics.renderMs = tickRender.start();

    auto camera = Camera::Main();
    if (camera) {
        // Preparação para renderização.
        RenderManager::Process();

        /* Preparação do shadow
        m_shadowbuffer->bind();
        GRender(GRENDERERDEPTH);
        GEnableDepthTest();
        GCullFaceFront();

        BatchingManager::OnRenderShadow();

        m_shadowbuffer->unbind();
        */

        // aplicação do anti-alising mass4.
        pScreenBuffer->bind();

        grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
        grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);
        grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_TRUE);

        RenderScene();

        // trocara para o frame buffer do pos processamento.
        pScreenBuffer->swap();

        pScreenBuffer->unbind();
    }

    #if !IS_GAME
    pImguibuffer->bind();
    #else
    GSetViewport(Screen::width, Screen::height);
    #endif

    // renderização da cena 2d e utilização do pos processamento;
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_FALSE);
    grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_FALSE);

    pScreenRender->render(pScreenBuffer->GetTexture());

    #if !IS_GAME
    pImguibuffer->unbind();
    #endif
    
    tickRender.end();
}

void Engine::RenderScene() {
    RenderManager::Renderer();

    DebugDraw::Renderer();

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    m_skybox->render();
    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LESS);
}

#if !IS_GAME
void Engine::UIFramebuffer() {
    Vector2 scale(Screen::width, Screen::height);
    
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void*)&scale);

    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);

    pImGuiRender->Begin();
    m_game->Gui();
    pImGuiRender->End();
}
#endif

void Engine::SetupEnable() {
    grr::gRender::SetRenderState(grr::GR_MULTISAMPLE, grr::GR_TRUE);

    grr::gRender::SetRenderState(grr::GR_CULL_FACE, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

