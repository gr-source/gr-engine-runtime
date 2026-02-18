#include "gr-base/core/thread/threadpool.hpp"

#include <gr-math/gmath.hpp>

#include <gr-render/gRender.h>

#include "gr-engine/settings.h"
#include "gr-engine/engine.h"

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
}

static Vector2 current_scroll_offset = Vector2::zero;
static Vector2 mouse_scroll = Vector2::zero;

void GLFWscrollfunCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    current_scroll_offset.x += static_cast<float>(xoffset);
    current_scroll_offset.y += static_cast<float>(yoffset);
}

void GLFWdropfunCallback(GLFWwindow* window, int path_count, const char* paths[])
{
    for (int i=0;i<path_count;i++)
    {
        // Engine::HandleEvent(Engine::DROFUNC, reinterpret_cast<const void *>(paths[i]));
    }
}

void glfwInstallCallback(GLFWwindow * window) {
    glfwSetFramebufferSizeCallback(window, GLFWframebuffersizefunCallback);

    glfwSetScrollCallback(window, GLFWscrollfunCallback);

    glfwSetDropCallback(window, GLFWdropfunCallback);
}

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
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

static Vector2 last_mouse_pos = Vector2::zero;
static Vector2 mouse_delta = Vector2::zero;

static Vector2 glfw_get_mouse_delta()
{
    return mouse_delta;
}

static Vector2 glfw_get_mouse_scroll()
{
    return mouse_scroll;
}

static void update_input_states()
{
    Vector2 current_pos = glfw_get_cursor_pos();
    
    mouse_delta.x = current_pos.x - last_mouse_pos.x;
    mouse_delta.y = current_pos.y - last_mouse_pos.y;

    last_mouse_pos = current_pos;

    mouse_scroll = current_scroll_offset;
    current_scroll_offset = Vector2::zero;
}

static void config_set_engine_callback()
{
    // ********** Window ********** //
    Window::SetWindowTitle  = &glfw_set_window_title;

    Window::SetWindowSize   = &glfw_set_window_size;

    Window::GetWindowSize   = &glfw_get_window_size;

    // ********** Input ********** //
    Input::IsKeyPress       = &glfw_is_key_press;
    
    Input::SetCursorPos     = &glfw_set_cursor_pos;
    Input::GetCursorPos     = &glfw_get_cursor_pos;

    Input::IsMousePress     = &glfw_get_mouse_button;

    Input::GetMouseDelta    = &glfw_get_mouse_delta;
    Input::GetMouseScroll   = &glfw_get_mouse_scroll;
}

static void destroy() {
    Settings::StopPlayEngine();

    auto& engine = Engine::GetInstance();
    engine.shutdown();
    
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

#include "gr-engine/core/time.hpp"

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

    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "GLFW not initialized." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window(CreateWindown("GR - GameEngine", SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, true), glfwDestroyWindow);
    if (!window)
    {
        std::cout << "Falied to create window." << std::endl;
        return 2;
    }

    glfwMakeContextCurrent(&(*window));

    glfwSwapInterval(0);

    glfwInstallCallback(&(*window));

    config_set_engine_callback();

    // glfwSetWindowPos(&(*window), 0, 0);

    if (!grr::gRender::Initialize())
    {
        window = nullptr;

        glfwTerminate();
        std::cout << "Falied to initialize opengl library." << std::endl;
        return 3;
    }

    Threadpool::Initialize();

    auto &engine = Engine::GetInstance();
    engine.init();

    GLFWframebuffersizefunCallback(&(*window), SCREEN_WIDTH, SCREEN_HEIGHT);

    gtimer time(60.0f);

    srand(*(int*)(void*)&Settings::IconSize);

    while (Settings::IsEngineRunning() && !glfwWindowShouldClose(&(*window)))
    {
        time.begin_frame();

        Clock::deltaTime = time.get_delta();
        Clock::time += Clock::deltaTime;

        glfwSwapBuffers(&(*window));
        glfwPollEvents();

        update_input_states();

        engine.process();

        time.end_frame();
    }

    window = nullptr;

    destroy();

    Threadpool::Shutdown();
}




