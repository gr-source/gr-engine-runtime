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

#include <imgui_internal.h>

std::thread Engine::m_thread[2];

Vector2 Engine::s_viewportSize = {};

GameObject* Engine::s_gameObjectSelected = nullptr;

std::unique_ptr<ImGuiRender> Engine::m_imGuiRender = nullptr;

void Engine::GRWindowResize(u32 width, u32 height) {
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
        Settings::IsCursorLock = !Settings::IsCursorLock;
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

void Engine::OnInit() {
    auto mainCamera = SceneManager::add("MainCamera");

    auto& transform = mainCamera->getTransform();
    transform.position = {0.0f, 0.0f, -5.0f};

    mainCamera->addComponent<Camera>();
    mainCamera->setActive(true);

    auto chunckManager = SceneManager::add("Chunkmanager");
    chunckManager->addComponent<ChunkManager>();

    /*
    auto cube = SceneManager::add("Cube");
    cube->setActive(true);

    auto& meshFilter = cube->addComponent<MeshFilter>();

    auto& mesh = meshFilter.getMesh();

    mesh.insert(indices3D);
    mesh.insert(vertex3D);

    auto& meshRender = cube->addComponent<MeshRender>();
    auto& material = meshRender.addMaterial("default");
    material.albedo = {1.0f, 0.0f, 1.0f, 1.0f};
    material.ao = 1.0f;
    material.roughness = 0.32f;
    material.metallic = 0.2f;

    meshRender.buildMesh();
    */

    std::string vert = Filesystem::getfilecontent(Filesystem::getpublic(Filesystem::eShader).string() + "/default.vert");
    std::string frag = Filesystem::getfilecontent(Filesystem::getpublic(Filesystem::eShader).string() + "/default.frag");

    m_framebuffer = Framebuffer::Create(1, 1);
    
    shader = grr::gShader::Create({frag.c_str()}, {vert.c_str()});

    EditorController::Initialize();

    m_imGuiRender = std::make_unique<ImGuiRender>("#version 330 core");
}

void Engine::OnUpdate() {
    if (Settings::IsCursorLock) {
        EditorController::Update();
    }

    EntityManager::OnUpdate();
}

void Engine::OnRender() {
    auto camera = Camera::GetCurrent();
    if (!camera || !shader->isValid()) {
        grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void*)&Color::red);
        grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);
        return;
    }

    m_framebuffer->bind();
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

    OnImGuiRender(camera);
    
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

    shader->unbind();
}

void Engine::OnImGuiRender(Camera *camera) {
    // ImGui Docking
    if (!IsEnableDocking()) {
        return;
    }

    // Window - hierarchy
    OnSetupUIHierarchy();

    // Window - viewport
    OnSetupUIViewport(camera);

    // Window - viewport
    OnSetupProperty();
}

void Engine::OnSetupUIViewport(Camera *camera) {
    ImGui::Begin("Viewport");

    auto avail = ImGui::GetContentRegionAvail();

    if (s_viewportSize.x != avail.x || s_viewportSize.y != avail.y) {
        camera->setAspectRation(avail.x / avail.y);

        m_framebuffer->updateViewport(avail.x, avail.y);

        Screen::SetResolution(avail.x, avail.y);

        s_viewportSize = {avail.x, avail.y};
    }

    ImGui::Image(reinterpret_cast<ImTextureID>(m_framebuffer->getTexture()->getID()), ImVec2(avail.x, avail.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

    ImGui::End();
}

bool DragFloatXYZ(const std::string& name, float* data) {
    ImGui::Text("%s", name.c_str());

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

    bool isChanged = false;

    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "X");
    ImGui::SameLine();
    if (ImGui::DragFloat(("##DragFloatX_" + name).c_str(), &data[0])) {
        isChanged = true;
    }

    ImGui::PopItemWidth();

    ImGui::SameLine();

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Y");
    ImGui::SameLine();
    if (ImGui::DragFloat(("##DragFloatY_" + name).c_str(), &data[1])) {
        isChanged = true;
    }
    
    ImGui::PopItemWidth();

    ImGui::SameLine();

    ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Z");
    ImGui::SameLine();
    if (ImGui::DragFloat(("##DragFloatZ_" + name).c_str(), &data[2])) {
        isChanged = true;
    }
    ImGui::PopItemWidth();

    return isChanged;
}

void OnSetupComponent(Transform* transform) {
    auto isOpen = ImGui::TreeNode("Transform");

    if (isOpen) {
        DragFloatXYZ("Posição", reinterpret_cast<float*>(&transform->position));

        DragFloatXYZ("Escala", reinterpret_cast<float*>(&transform->scale));

        auto& rotation = transform->rotation;

        auto euler = rotation.eulerAngles();
        if (DragFloatXYZ("Rotação", reinterpret_cast<float*>(&euler))) {
            rotation = Math::normalize(Math::euler({Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)}));
        }
        ImGui::TreePop();
    }
}

void Engine::OnSetupProperty() {
    ImGui::Begin("Property");
    if (s_gameObjectSelected) {
        auto name = s_gameObjectSelected->getName();

        ImGui::SeparatorText(name.c_str());

        s_gameObjectSelected->RenderProperty<Transform>(OnSetupComponent);
    }
    ImGui::End();
}


void Engine::OnSetupUIHierarchy() {
    ImGui::Begin("Hierarchy");
    
    auto name = SceneManager::GetName();

    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

    auto isOpen = ImGui::TreeNode(name.c_str());

    if (isOpen) {
        auto objects = SceneManager::GetObjects();

        std::for_each(objects.begin(), objects.end(), ForechSceneGameObject);

        ImGui::TreePop();
    }
    ImGui::End();
}

void Engine::ForechSceneGameObject(GameObject *gameObject) {
    auto transform = gameObject->getTransform();

    auto name = gameObject->getName();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_Leaf;

    if (transform.hasParent()) {
        flags =~ ImGuiTreeNodeFlags_Leaf;
    }

    if (s_gameObjectSelected == gameObject) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);
    if (ImGui::IsItemClicked()) {
        s_gameObjectSelected = gameObject;
    }

    if (isOpen) {
        ImGui::TreePop();
    }
}

const bool Engine::IsEnableDocking() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("#GRGameEngine3D", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(ImGui::GetID("##RenderDocking"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return true;
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
    if (shader) {
        shader->destroy();
    }

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


