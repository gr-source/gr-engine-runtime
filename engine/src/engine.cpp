#include "engine.h"
#include "input.h"
#include "screen.h"
#include "random.hpp"
#include "taskRuntime.h"


#include "editor/editorController.hpp"

#include "component/cameraController.h"
#include "component/MeshRender.h"
#include "component/meshFilter.h"
#include "component/Transform.h"
#include "component/camera.h"

#include "ecs/entityManager.h"

#include "ui/ImGuiRender.h"

#include "framebuffer/screenbuffer.h"
#include "framebuffer/skyboxbuffer.h"
#include "framebuffer/shadowbuffer.h"
#include "framebuffer/imguibuffer.h"
#include "framebuffer/gBuffer.h"

#include "resources/resources.h"

#include "physics/physicsScene.h"

#include "filesystem/stream.h"

#include "core/scene/SceneManager.h"
#include "renderManager.h"
#include "objectManager.h"
#include "eventManager.h"
#include "screenRender.h"
#include "skybox.h"
#include "Global.h"
#include "timer.h"

#include "thread.h"
#include "settings.h"

#include <linux/input-event-codes.h>
#include <filesystem>

#include <gr-render/gRender.h>
#include <gr-render/gShader.h>

std::thread Engine::m_thread[2];

std::vector<Vertex3D> vertex3D {
    {Vector3(-1.0f, -1.0f,  1.0f), Vector3(0.0f, 0.0f,  1.0f), Vector2(0.0f, 0.0f)},
    {Vector3(-1.0f,  1.0f,  1.0f), Vector3(0.0f, 0.0f,  1.0f), Vector2(0.0f, 1.0f)},
    {Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f,  1.0f), Vector2(1.0f, 0.0f)},
    {Vector3(-1.0f,  1.0f, -1.0f), Vector3(0.0f, 0.0f,  1.0f), Vector2(1.0f, 1.0f)},

    {Vector3( 1.0f, -1.0f,  1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f)},
    {Vector3( 1.0f,  1.0f,  1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f)},
    {Vector3( 1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f)},
    {Vector3( 1.0f,  1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f)}
};

std::vector<u32> indices3D {
    1, 2, 0,
    3, 6, 2,
    7, 4, 6,
    5, 0, 4,
    6, 0, 2,
    3, 5, 7,
    1, 3, 2,
    3, 7, 6,
    7, 5, 4,
    5, 1, 0,
    6, 4, 0,
    3, 1, 5
};

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
    switch (key) {
    case KEY_ESC: {
        Settings::IsEngineRunning = false;
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
        // EditorController::movementSpeed += 5.0f;
    } else if (yoffset == 1.0f) {
        // EditorController::movementSpeed -= 5.0f;
    }
}

void Engine::GRDropFunc(const char *path) {
}

grr::gShader* shader = nullptr;

GameObject* CreateCube(u8 index, const Vector3& position) {
    auto cube = SceneManager::add("Cube[" + std::to_string(index) + "]");

    auto& transform = cube->getTransform();
    transform.position = position;

    auto& meshFilter = cube->addComponent<MeshFilter>();

    auto& mesh = meshFilter.getMesh();
    mesh.insert(vertex3D);
    mesh.insert(indices3D);

    auto& meshRender = cube->addComponent<MeshRender>();
    auto& material = meshRender.addMaterial("material");
    material.albedo = {1.0f, 0.0f, 1.0f, 1.0f};
    material.metallic = 0.5f;
    material.roughness = 0.32f;
    material.ao = 0.5f;
    return cube;
}

GameObject* cube = nullptr;

void Engine::OnInit() {
    auto mainCamera = SceneManager::add("MainCamera");

    auto& transform = mainCamera->getTransform();
    transform.position = {0.0f, 0.0f, -5.0f};

    mainCamera->addComponent<Camera>();
    mainCamera->setActive(true);

    Random::SetSeed(std::time(nullptr));

    cube = CreateCube(0, Vector3::zero);

    std::string vert = Filesystem::getfilecontent(Filesystem::getpublic(Filesystem::eShader).string() + "/default.vert");
    std::string frag = Filesystem::getfilecontent(Filesystem::getpublic(Filesystem::eShader).string() + "/default.frag");

    shader = grr::gShader::Create({frag.c_str()}, {vert.c_str()});

    EditorController::Initialize();
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

    grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::black);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
    grr::gRender::SetRenderState(grr::GR_DEPTH, grr::GR_TRUE);

    OnPreRender(camera);
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
