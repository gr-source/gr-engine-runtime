#include "engine.h"
#include "input.h"
#include "screen.h"
#include "random.hpp"
#include "taskRuntime.h"

#include "editor/editorController.hpp"

#include "component/cameraController.h"
#include "component/chunkManager.hpp"
#include "component/MeshRender.h"
#include "component/meshFilter.h"
#include "component/Transform.h"
#include "component/terrain.hpp"
#include "component/camera.h"

#include "ecs/entityManager.h"

#include "ui/ImGuiRender.h"

#include "framebuffer/framebuffer.hpp"

#include "resources/resources.h"

#include "physics/physicsScene.h"

#include "filesystem/stream.h"

#include "scene/SceneManager.h"
#include "renderManager.h"
#include "objectManager.h"
#include "eventManager.h"
#include "screenRender.h"
#include "Global.h"
#include "timer.h"

#include "thread.h"
#include "settings.h"

#include <linux/input-event-codes.h>
#include <filesystem>

#include <gr-render/gFramebuffer.h>
#include <gr-render/gRender.h>
#include <gr-render/gShader.h>

std::thread Engine::m_thread[2];

std::unique_ptr<ImGuiRender> Engine::m_imGuiRender = nullptr;

void Engine::GRWindowResize(u32 width, u32 height) {
    Screen::SetResolution(width, height);
}

void Engine::MainLoop(std::function<void()> func) {
    m_thread[0] = std::thread(OnLoader);
    // m_thread[1] = std::thread(OnPhysics);

    Time timer(60.0f);
    while (Settings::IsEngineRunning) {
        Time::deltaTime = gameStatics.updateMs = timer.SleepTick();
        Time::FPS = timer.getFPS();

        Engine::OnUpdate();

        Engine::OnRender();

        EventManager::ProcessEvent();

        func();
    }
}

void Engine::GRKeyboardDown(u32 key) {
    auto& io = ImGui::GetIO();
    switch (key) {
    case KEY_ESC: {
        io.MouseDrawCursor = !io.MouseDrawCursor;
        // Settings::IsEngineRunning = false;
        break;
    }
    default:
        break;
    }
}

void Engine::GRKeyboardUp(u32 key) {
}

void Engine::GRCursorPos(float xpos, float ypos) {
}

void Engine::GRCursorScrool(float xoffset, float yoffset) {
    if (yoffset == -1.0f) {
        EditorController::s_camSpeed += 0.2f;
    } else if (yoffset == 1.0f) {
        EditorController::s_camSpeed -= 0.2f;
    }
}

void Engine::GRDropFunc(const char *path) {
}

grr::gShader* shader = nullptr;

Framebuffer* m_framebuffer = nullptr;

void Engine::OnInit() {
    auto mainCamera = SceneManager::add("MainCamera");

    auto& transform = mainCamera->getTransform();
    transform.position = {0.0f, 0.0f, -5.0f};

    mainCamera->addComponent<Camera>();
    mainCamera->setActive(true);

    auto chunckManager = SceneManager::add("Chunkmanager");
    chunckManager->addComponent<ChunkManager>();

    std::string vert = Filesystem::getfilecontent(Filesystem::getpublic(Filesystem::eShader).string() + "/default.vert");
    std::string frag = Filesystem::getfilecontent(Filesystem::getpublic(Filesystem::eShader).string() + "/default.frag");

    shader = grr::gShader::Create({frag.c_str()}, {vert.c_str()});

    EditorController::Initialize();

    m_imGuiRender = std::make_unique<ImGuiRender>("#version 330 core");

    m_framebuffer = Framebuffer::Create(Screen::width, Screen::height);
}

void Engine::OnUpdate() {
    EditorController::Update();

    EntityManager::OnUpdate();
}

void Engine::OnRender() {
    auto camera = Camera::GetCurrent();
    if (!camera) {
        grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::red);
        grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
        return;
    }

    m_framebuffer->bind(Screen::width, Screen::height);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::blue);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LESS);
    grr::gRender::SetRenderState(grr::GR_CULL_FACE, grr::GR_TRUE);
    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);

    OnPreRender(camera);

    m_framebuffer->unbind();

    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_FALSE);

    m_imGuiRender->Begin();

    ImGui::Begin("Viewport");

    auto avail = ImGui::GetContentRegionAvail();

    ImGui::Image(reinterpret_cast<ImTextureID>(m_framebuffer->getTexture()->getID()), ImVec2(avail.x, avail.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
    ImGui::End();
    m_imGuiRender->End();
}

void Engine::OnPostRender() {
}

void Engine::OnPreRender(Camera *camera) {
    shader->bind();

    grr::gShader::SetUniform("projection", 1, camera->getPerspesctive());
    grr::gShader::SetUniform("view", 1, camera->getViewMatrix());
    grr::gShader::SetUniform("viewPos", 1, camera->getPosition());

    RenderManager::OnRenderer(shader);
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
    if (m_framebuffer) {
        m_framebuffer->destroy();
    }

    if (m_thread[0].joinable()) {
        m_thread[0].join();
    }
    if (m_thread[1].joinable()) {
        m_thread[1].join();
    }
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


