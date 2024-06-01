#include "plataform.h"

#include "../engine.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

Window Plataform::m_window = nullptr;

void Plataform::Initialize(Monitor monitor) {
    if (glfwInit() != GLFW_TRUE) {
        Engine::SetError("glfw do not initialize.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(monitor.width, monitor.height, monitor.title.c_str(), nullptr, nullptr);
    if (!m_window) {
        Engine::SetError("Window do not created.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }
    glfwMakeContextCurrent((GLFWwindow*)m_window);

    if (glewInit() != GLEW_OK) {
        Engine::SetError("Glew do not initialize.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }

    glfwSetWindowPos((GLFWwindow*)m_window, static_cast<int>(monitor.x), static_cast<int>(monitor.y));

    Engine::SetHandleEvent(Engine::GRINITIALIZEOPENGL);

    MainLoop();
}

void Plataform::MainLoop() {
    if (!m_window) {
        Engine::SetError("Windown is null.");
        Engine::SetHandleEvent(Engine::GRERROR);
        return Destroy();
    }

    while (!glfwWindowShouldClose((GLFWwindow*)m_window)) {
        Engine::SetHandleEvent(Engine::GRUPDATE);

        Engine::SetHandleEvent(Engine::GRRENDERER);

        glfwPollEvents();
        glfwSwapBuffers((GLFWwindow*)m_window);
    }

    Destroy();
}

void Plataform::Destroy() {
    if (m_window) {
        glfwDestroyWindow((GLFWwindow*)m_window);
    }

    glfwTerminate();

    Engine::SetHandleEvent(Engine::GRDESTROY);
}
