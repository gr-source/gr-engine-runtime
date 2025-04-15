#include <settings.h>
#include <engine.h>

#include <gr-base/core/window.hpp>
#include <gr-base/core/input.hpp>

#include <GLFW/glfw3.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600

std::unordered_map<int, grm::u32> m_keyMap = {
    {GLFW_KEY_ESCAPE, KEY_ESCAPE},
    {GLFW_KEY_W, KEY_W},
    {GLFW_KEY_S, KEY_S},
    {GLFW_KEY_A, KEY_A},
    {GLFW_KEY_D, KEY_D}
};

GLFWwindow *CreateWindown(const char* name, int width, int height, GLFWwindow * share, bool isVisible) {
    glfwWindowHint(GLFW_VISIBLE, isVisible ? GL_TRUE : GL_FALSE);

    return glfwCreateWindow(width, height, name, nullptr, share);
}

void GLFWframebuffersizefunCallback(GLFWwindow* window, int width, int height) {
    const grm::u32 scale[2]{static_cast<const grm::u32>(width), static_cast<const grm::u32>(height)};
    Engine::HandleEvent(Engine::GR_RESIZE, scale);
}

void GLFWkeyfunCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)
        Engine::HandleEvent(Engine::GR_KEYBOARD_DOWN, reinterpret_cast<const void*>(&m_keyMap[key]));
    
    if (action == GLFW_RELEASE)
        Engine::HandleEvent(Engine::GR_KEYBOARD_UP, reinterpret_cast<const void*>(&m_keyMap[key]));
}

void GLFWcursorposfunCallback(GLFWwindow* window, double xpos, double ypos) {
    const float pos[2] {static_cast<const float>(xpos), static_cast<const float>(ypos)};

    Engine::HandleEvent(Engine::GR_CURSOR_POS, pos);
}

void GLFWscrollfunCallback(GLFWwindow* window, double xoffset, double yoffset) {
    const float offset[2] {static_cast<const float>(xoffset), static_cast<const float>(yoffset)};

    Engine::HandleEvent(Engine::GR_CURSOR_SCROLL, offset);
}

void GLFWdropfunCallback(GLFWwindow* window, int path_count, const char* paths[]) {
    for (int i=0;i<path_count;i++) {
        Engine::HandleEvent(Engine::GR_DROFUNC, reinterpret_cast<const void *>(paths[i]));
    }
}

void glfwInstallCallback(GLFWwindow * window) {
    glfwSetFramebufferSizeCallback(window, GLFWframebuffersizefunCallback);

    glfwSetKeyCallback(window, GLFWkeyfunCallback);

    glfwSetCursorPosCallback(window, GLFWcursorposfunCallback);

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

static void glfw_set_window_size(const gVector2<int> &size)
{
    glfwSetWindowSize(glfwGetCurrentContext(), size.x, size.y);
}

static gVector2<int> glfw_get_window_size()
{
    gVector2<int> data = {0, 0};
    glfwGetWindowSize(glfwGetCurrentContext(), &data.x, &data.y);
    return data;
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

    Engine::HandleEvent(Engine::GR_DESTROY, nullptr);
    
    glfwTerminate();
}

#include <sys/signal.h>

void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
    printf("Caught segfault at address %p\n", si->si_addr);

    destroy();
    
    exit(1);
}

int main(int argc, char *argv[]) {
    // ===============================
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

    Engine::HandleEvent(Engine::GR_INITIALIZE, nullptr);

    GLFWframebuffersizefunCallback(&(*window), SCREEN_WIDTH, SCREEN_HEIGHT);

    while (Settings::IsEngineRunning() && !glfwWindowShouldClose(&(*window))) {
        glfwSwapBuffers(&(*window));
        glfwPollEvents();

        Engine::HandleEvent(Engine::GR_IDLE, nullptr);
    }

    window = nullptr;

    destroy();
}




