#include "plataform.h"

#include <gRender.h>

#include "../engine.h"
#include "../screen.h"
#include "../timer.h"

#include <GLFW/glfw3.h>

Window Plataform::m_window = nullptr;

Monitor Plataform::m_monitor;

void Plataform::Initialize() {
    if (glfwInit() != GLFW_TRUE) {
        Engine::SetError("glfw do not initialize.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_monitor.width, m_monitor.height, m_monitor.title.c_str(), nullptr, nullptr);
    if (!m_window) {
        Engine::SetError("Window do not created.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }
    glfwMakeContextCurrent((GLFWwindow*)m_window);

    if (!gr::gRender::OpenGLInit()) {
        Engine::SetError("Glew do not initialize.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }
    Screen::SetResolution(m_monitor.width, m_monitor.height);

    gr::gRender::SetResolution(m_monitor.width, m_monitor.height);

    glfwSetWindowPos((GLFWwindow*)m_window, static_cast<int>(m_monitor.x), static_cast<int>(m_monitor.y));
    
    glfwSetFramebufferSizeCallback((GLFWwindow*)m_window, [](GLFWwindow* window, int width, int height) {
        Screen::SetResolution(static_cast<u32>(width), static_cast<u32>(height));
        gr::gRender::SetResolution(static_cast<u32>(width), static_cast<u32>(height));
        Engine::SetHandleEvent(Engine::GRRESIZE);
    });

    Engine::SetHandleEvent(Engine::GRINITIALIZEOPENGL);

    MainLoop();
}

Monitor &Plataform::GetMonitor() {
    return m_monitor;
}

void Plataform::MainLoop() {
    if (!m_window) {
        Engine::SetError("Windown is null.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }

    Time timer(60.0f);
    while (!glfwWindowShouldClose((GLFWwindow*)m_window)) {
        Time::deltaTime = timer.SleepTick();

        Engine::SetHandleEvent(Engine::GRUPDATE);

        Engine::SetHandleEvent(Engine::GRRENDERER);

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

    Engine::SetHandleEvent(Engine::GRDESTROY);
}
