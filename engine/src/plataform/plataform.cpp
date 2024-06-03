#include "plataform.h"

#include <gRender.h>

#include "../engine.h"
#include "../screen.h"
#include "../timer.h"
#include "../input.h"

#include <GLFW/glfw3.h>

#include <linux/keyboard.h>

Window Plataform::m_window = nullptr;

std::unordered_map<u32, u32> Plataform::m_keyMap {
    {GLFW_KEY_A, KEY_A},
    {GLFW_KEY_W, KEY_W},
    {GLFW_KEY_S, KEY_S},
    {GLFW_KEY_D, KEY_D},
    {GLFW_KEY_SPACE, KEY_SPACE},
    {GLFW_KEY_ESCAPE, KEY_ESC},
    {GLFW_KEY_ESCAPE, KEY_ESC},
    {GLFW_RELEASE, BTN_GEAR_UP},
    {GLFW_PRESS, BTN_GEAR_DOWN},
    {GLFW_REPEAT, KEY_REDO},
};

void Plataform::Initialize(const GWindow& window) {
    if (glfwInit() != GLFW_TRUE) {
        Engine::SetError("glfw do not initialize.");
        Engine::SetHandleEvent(Engine::GR_ERROR);
        return Destroy();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(window.width, window.height, window.title.c_str(), nullptr, nullptr);
    if (!m_window) {
        Engine::SetError("Window do not created.");
        Engine::SetHandleEvent(Engine::GR_ERROR);
        return Destroy();
    }
    glfwMakeContextCurrent((GLFWwindow*)m_window);

    if (!gr::gRender::OpenGLInit()) {
        Engine::SetError("Glew do not initialize.");
        Engine::SetHandleEvent(Engine::GR_ERROR);
        return Destroy();
    }
    Screen::SetResolution(window.width, window.height);

    gr::gRender::SetResolution(window.width, window.height);

    glfwSetWindowPos((GLFWwindow*)m_window, static_cast<int>(window.x), static_cast<int>(window.y));
    
    glfwSetFramebufferSizeCallback((GLFWwindow*)m_window, [](GLFWwindow* window, int width, int height) {
        Screen::SetResolution(static_cast<u32>(width), static_cast<u32>(height));
        gr::gRender::SetResolution(static_cast<u32>(width), static_cast<u32>(height));
        Engine::SetHandleEvent(Engine::GR_RESIZE);
    });

    glfwSetKeyCallback((GLFWwindow*)m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        GParams params;
        params.keyboard.key = m_keyMap[key];
        params.keyboard.action = m_keyMap[action];

        Engine::SetHandleEvent(Engine::GR_KEYBOARD, &params);
    });

    glfwSetMouseButtonCallback((GLFWwindow*)m_window, [](GLFWwindow* window, int button, int action, int mods) {
        //
    });

    glfwSetCursorPosCallback((GLFWwindow*)m_window, [](GLFWwindow* window, double xpos, double ypos) {
        GParams params;
        params.mouse = {static_cast<float>(xpos), static_cast<float>(ypos)};

        Engine::SetHandleEvent(Engine::GR_MOUSE_POS, &params);
    });

    Engine::SetHandleEvent(Engine::GR_INITIALIZE);

    MainLoop();
}

void Plataform::MainLoop() {
    if (!m_window) {
        Engine::SetError("Windown is null.");
        Engine::SetHandleEvent(Engine::GR_ERROR);
        return Destroy();
    }

    Time timer(60.0f);
    while (!glfwWindowShouldClose((GLFWwindow*)m_window)) {
        Time::deltaTime = timer.SleepTick();

        Engine::SetHandleEvent(Engine::GR_UPDATE);

        Engine::SetHandleEvent(Engine::GR_RENDERER);

        glfwPollEvents();
        glfwSwapBuffers((GLFWwindow*)m_window);
    }

    Destroy();
}

void Plataform::Destroy() {
    gr::gRender::OpenGLShutsown();
    
    if (m_window) {
        glfwDestroyWindow((GLFWwindow*)m_window);
    }

    glfwTerminate();

    Engine::SetHandleEvent(Engine::GR_DESTROY);
}
