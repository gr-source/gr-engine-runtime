#include "engine.h"
#include "input.h"
#include "screen.h"

#include "component/MeshRender.h"

#include "ecs/entityManager.h"

#include "ui/ImGuiRender.h"

#include "framebuffer/screenbuffer.h"
#include "framebuffer/skyboxbuffer.h"
#include "framebuffer/shadowbuffer.h"
#include "framebuffer/imguibuffer.h"
#include "framebuffer/gBuffer.h"

#include "resources/resources.h"

#include "physics/physicsScene.h"

#include "batchingManager.h"
#include "renderManager.h"
#include "objectManager.h"
#include "eventManager.h"
#include "screenRender.h"
#include "renderer.h"
#include "skybox.h"
#include "Global.h"
#include "timer.h"

#include "thread.h"
#include "settings.h"

void engine::OnResizeWindow(u32 width, u32 height) {
    Screen::SetResolution(width, height);
}

void engine::MainLoop(std::function<void()> func) {
    m_thread[0] = std::thread(OnLoader);
    m_thread[1] = std::thread(OnPhysics);

    Time timer(60.0f);
    while (Settings::IsEngineRunning) {
        Time::deltaTime = gameStatics.updateMs = timer.SleepTick();
        Time::FPS = timer.getFPS();

        engine::OnUpdate();

        engine::OnRender();

        func();
    }
}

void engine::Escap() {
    #if IS_GAME
    Settings::IsGameRunning = !Settings::IsGameRunning;

    if (Settings::IsGameRunning) {
        glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    #else
    ImGuiIO& io = ImGui::GetIO();

    io.MouseDrawCursor = !io.MouseDrawCursor;
    #endif
}

void engine::WindowInstance(void *instance) {
    Input::SetInstance(instance);
}

std::unique_ptr<Game> engine::m_game = nullptr;

Skybox* engine::m_skybox = nullptr;

std::unique_ptr<ScreenRender> pScreenRender = nullptr;

std::unique_ptr<ImGuiRender> pImGuiRender = nullptr;

Shadowbuffer* m_shadowbuffer = nullptr;

ScreenBuffer* pScreenBuffer = nullptr;

Imguibuffer* pImguibuffer = nullptr;

GBuffer* m_gbuffer = nullptr;

std::thread engine::m_thread[2];

void engine::OnInit() {
    pImGuiRender = std::make_unique<ImGuiRender>("#version 330 core");

    pScreenBuffer = &ScreenBuffer::getInstance();
    pImguibuffer = &Imguibuffer::getInstance();

    DebugDraw::initialize();

    pScreenRender = std::make_unique<ScreenRender>();

    m_shadowbuffer = &Shadowbuffer::getInstance();

    m_skybox = &Skybox::getInstance();

    m_game = std::make_unique<Game>();

    SetupEnable();

    RenderManager::Initialize();

    #if IS_GAME
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    #endif
}

void engine::OnUpdate() {
    EntityManager::OnUpdate();

    m_game->Update();
}

void engine::OnRender() {
    EventManager::ProcessEvent();

    MainFramebuffer();

    #if !IS_GAME
    UIFramebuffer();
    #endif
}

void engine::OnPhysics() {
    PhysicsScene&  pPhysicsScene  = PhysicsScene::getInstance();
    Time timer(60.0f);

    do {
        gameStatics.physicsMs = timer.SleepTick();

        if (Settings::IsGameRunning)
            pPhysicsScene.Simulate(gameStatics.physicsMs);
    } while (Settings::IsEngineRunning);
}

void engine::OnLoader() {
    Resources::Initialize();
    
    Time timer(5.0f);
    
    do {
        timer.SleepTick();
        Resources::OnThread();
    } while (Settings::IsEngineRunning);
}

void engine::OnDestroy() {
    if (m_thread[0].joinable()) {
        m_thread[0].join();
    }
    if (m_thread[1].joinable()) {
        m_thread[1].join();
    }
}

Time tickRender;
void engine::MainFramebuffer() {
    gameStatics.renderMs = tickRender.start();

    // Preparação ara renderização.
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

    GRender(GRENDERER3D);
    GEnableDepthTest();
    GEnableFrameBufferSRGB();
    GCullFaceBack();

    RenderScene();

    // trocara para o frame buffer do pos processamento.
    pScreenBuffer->swap();

    pScreenBuffer->unbind();

    #if !IS_GAME
    pImguibuffer->bind();
    #else
    GSetViewport(Screen::width, Screen::height);
    #endif

    // renderização da cena 2d e utilização do pos processamento;
    GRender(GRENDERER2D);
    GDisableDepthTest();
    GDisableFrameBufferSRGB();

    pScreenRender->render(pScreenBuffer->GetTexture());

    #if !IS_GAME
    pImguibuffer->unbind();
    #endif
    
    tickRender.end();
}

void engine::RenderScene() {
    DebugDraw::Renderer();

    RenderManager::Renderer();

    glDepthFunc(GL_LEQUAL);
    m_skybox->render();
    glDepthFunc(GL_LESS);
}

#if !IS_GAME
void engine::UIFramebuffer() {
    GSetViewport(Screen::width, Screen::height);

    GClearColor(Color::black);
    GRender(GRENDERER2D);

    pImGuiRender->Begin();
    m_game->Gui();
    pImGuiRender->End();
}
#endif

void engine::SetupEnable() {
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
