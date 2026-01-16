#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"
#include "gr-base/script/camera.hpp"

#include <gr-math/gmath.hpp>

#include <gr-render/gRender.h>

#include <settings.h>
#include <engine.h>

#include <gr-base/core/window.hpp>
#include <gr-base/core/input.hpp>

#include <string.h>

#include <GLFW/glfw3.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600

GLFWwindow *CreateWindown(const char* name, int width, int height, GLFWwindow * share, bool isVisible)
{
    glfwWindowHint(GLFW_VISIBLE, isVisible ? GL_TRUE : GL_FALSE);

    return glfwCreateWindow(width, height, name, nullptr, share);
}

void GLFWframebuffersizefunCallback(GLFWwindow* window, int width, int height)
{
    const u32 scale[2]{static_cast<const u32>(width), static_cast<const u32>(height)};
    Engine::HandleEvent(Engine::RESIZE, scale);
}

void GLFWscrollfunCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Vector2 offset{static_cast<float>(xoffset), static_cast<float>(yoffset)};
}

void GLFWdropfunCallback(GLFWwindow* window, int path_count, const char* paths[])
{
    for (int i=0;i<path_count;i++)
    {
        Engine::HandleEvent(Engine::DROFUNC, reinterpret_cast<const void *>(paths[i]));
    }
}

void glfwInstallCallback(GLFWwindow * window) {
    glfwSetFramebufferSizeCallback(window, GLFWframebuffersizefunCallback);

    glfwSetScrollCallback(window, GLFWscrollfunCallback);

    glfwSetDropCallback(window, GLFWdropfunCallback);
}

void error_callback(int error, const char* description) {
    std::cout << "Error: " << description << std::endl;
}

static void glfw_set_window_title(const char *title)
{
    glfwSetWindowTitle(glfwGetCurrentContext(), title);
}

static void glfw_set_window_size(const Vector2 &size)
{
    glfwSetWindowSize(glfwGetCurrentContext(), size.x, size.y);
}

static Vector2 glfw_get_window_size()
{
    int x, y;
    glfwGetWindowSize(glfwGetCurrentContext(), &x, &y);
    return Vector2{static_cast<float>(x), static_cast<float>(y)};
}

// ********** Input ********** //
static bool glfw_is_key_press(int key)
{
    return glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS;
}

static void glfw_set_cursor_pos(const Vector2 &pos)
{
    glfwSetCursorPos(glfwGetCurrentContext(), static_cast<double>(pos.x), static_cast<double>(pos.y));
}

static Vector2 glfw_get_cursor_pos()
{
    double x = 0.0, y = 0.0;
    glfwGetCursorPos(glfwGetCurrentContext(), &x, &y);
    return Vector2{static_cast<float>(x), static_cast<float>(y)};
}

static bool glfw_get_mouse_button(int button)
{
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_PRESS;
}

static void config_set_engine_callback()
{
    Window::SetWindowTitle  = &glfw_set_window_title;

    Window::SetWindowSize   = &glfw_set_window_size;

    Window::GetWindowSize   = &glfw_get_window_size;

    // ********** Input ********** //
    Input::IsKeyPress       = &glfw_is_key_press;
    
    Input::SetCursorPos     = &glfw_set_cursor_pos;
    Input::GetCursorPos     = &glfw_get_cursor_pos;

    Input::IsMousePress     = &glfw_get_mouse_button;
}

static void destroy() {
    Settings::StopPlayEngine();

    Engine::HandleEvent(Engine::DESTROY, nullptr);
    
    glfwTerminate();
}

#include <sys/signal.h>

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
    printf("Caught segfault at address %p\n", si->si_addr);

    destroy();
    
    exit(1);
}

#include <gr-base/core/clock.hpp>
#include "core/time.hpp"

#include <gr-base/core/gameObject.hpp>

int main(int argc, char *argv[])
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);

    // ===============================
    
    glfwSetErrorCallback(error_callback);

    if (glfwInit() != GLFW_TRUE) {
        std::cout << "GLFW not initialized." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window(CreateWindown("GR - GameEngine", SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, true), glfwDestroyWindow);
    if (!window) {
        std::cout << "Falied to create window." << std::endl;
        return 2;
    }

    glfwMakeContextCurrent(&(*window));

    glfwSwapInterval(0);

    glfwInstallCallback(&(*window));

    config_set_engine_callback();

    // glfwSetWindowPos(&(*window), 0, 0);

    if (!grr::gRender::Initialize()) {
        window = nullptr;

        glfwTerminate();
        std::cout << "Falied to initialize opengl library." << std::endl;
        return 3;
    }

    Threadpool::Initialize();

    Engine::HandleEvent(Engine::INITIALIZE, nullptr);

    GLFWframebuffersizefunCallback(&(*window), SCREEN_WIDTH, SCREEN_HEIGHT);

    gtimer time;

    while (Settings::IsEngineRunning() && !glfwWindowShouldClose(&(*window)))
    {
        Clock::deltaTime = time.get_delta();

        glfwSwapBuffers(&(*window));
        glfwPollEvents();

        Engine::HandleEvent(Engine::LOOP, nullptr);

        time.wait();
    }

    window = nullptr;

    destroy();

    Threadpool::Shutdown();
}




