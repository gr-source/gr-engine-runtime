#include "engine.h"
#include "input.h"
#include "screen.h"

#include "component/MeshRender.h"

#include "ecs/EntityManager.h"

#include "ui/ImGuiRender.h"

#include "framebuffer/screenbuffer.h"
#include "framebuffer/skyboxbuffer.h"
#include "framebuffer/shadowbuffer.h"
#include "framebuffer/imguibuffer.h"
#include "framebuffer/gBuffer.h"

#include "resources/resources.h"

#include "physics/physicsScene.h"

#include "batchingManager.h"
#include "objectManager.h"
#include "eventManager.h"
#include "screenRender.h"
#include "renderer.h"
#include "skybox.h"
#include "Global.h"
#include "timer.h"

#include "thread.h"
#include "settings.h"

void engine::GLFWkeyfunCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Input::SetKeyAction(getKeyCode(static_cast<unsigned char>(key)), getKeyAction(action));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
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
}

void engine::GLFWframebuffersizefunCallback(GLFWwindow* window, int width, int height) {
    Screen::SetResolution(width, height);
}

void engine::GLFWscrollfun(GLFWwindow* window, double xoffset, double yoffset) {
    // std::cout << xoffset << " " << yoffset << std::endl;
}

void engine::GLFWcursorposfun(GLFWwindow* window, double xpos, double ypos) {
    Input::SetCursorPos(static_cast<float>(xpos), static_cast<float>(ypos));
}

void engine::GLFWcharfun(GLFWwindow* window, unsigned int codepoint) {
}

void engine::GLFWmousebuttonfun(GLFWwindow* window, int button, int action, int mods) {
    Input::SetMouseAction(Input::getMouseButton(button), getKeyAction(action));
}

void engine::GLFWcursorenterfun(GLFWwindow* window, int entered) {
}

void engine::GLFWdropfun(GLFWwindow *window, int path_count, const char *paths[]) {
    std::vector<std::filesystem::path> m_files;

    for (int i=0;i<path_count;i++) {
        m_files.push_back(paths[i]);
    }
    // Game::ImportFiles(m_files);
}

void engine::OnScreenSize(int width, int height) {
    Screen::SetResolution(static_cast<float>(width), static_cast<float>(height));
}

std::unique_ptr<Game> engine::m_game = nullptr;

Skybox* engine::m_skybox = nullptr;

std::unique_ptr<ScreenRender> pScreenRender = nullptr;

std::unique_ptr<ImGuiRender> pImGuiRender = nullptr;

Shadowbuffer* m_shadowbuffer = nullptr;

ScreenBuffer* pScreenBuffer = nullptr;

Imguibuffer* pImguibuffer = nullptr;

GBuffer* m_gbuffer = nullptr;

void engine::OnInit() {
    pImGuiRender = std::make_unique<ImGuiRender>("#version 330 core");

    pScreenBuffer = &ScreenBuffer::getInstance();
    pImguibuffer = &Imguibuffer::getInstance();
    m_gbuffer = &GBuffer::getInstance();

    DebugDraw::initialize();

    pScreenRender = std::make_unique<ScreenRender>();

    m_shadowbuffer = &Shadowbuffer::getInstance();

    m_skybox = &Skybox::getInstance();

    m_game = std::make_unique<Game>();

    SetupEnable();

    BatchingManager::Initialize();

    #if IS_GAME
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    #endif
}

Time timer_update(60.0f);
void engine::OnUpdate() {
    Time::deltaTime = gameStatics.updateMs = timer_update.SleepTick();

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

void engine::OnDestroy() {}

Time tickRender;
void engine::MainFramebuffer() {
    gameStatics.renderMs = tickRender.start();

    // Preparação ara renderização.
    BatchingManager::OnUpdate();

    // Preparação do shadow
    m_shadowbuffer->bind();
    GRender(GRENDERERDEPTH);
    GEnableDepthTest();
    GCullFaceFront();

    BatchingManager::OnRenderShadow();

    m_shadowbuffer->unbind();

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

    BatchingManager::OnRender();

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
