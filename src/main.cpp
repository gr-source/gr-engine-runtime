#include <settings.h>
#include <engine.h>
#include <input.h>

#include <linux/input-event-codes.h>

#include <GLFW/glfw3.h>

#define SCREEN_WIDTH 768
#define SCREEN_HEIGHT 512

std::unordered_map<int, u32> m_keyMap = {
    {GLFW_KEY_ESCAPE, KEY_ESC},
    {GLFW_KEY_W, KEY_W},
    {GLFW_KEY_S, KEY_S},
    {GLFW_KEY_A, KEY_A},
    {GLFW_KEY_D, KEY_D}
};

typedef GLFWwindow* Window;

Window window = nullptr;

Window CreateWindown(const char* name, int width, int height, Window share, bool isVisible) {
    if (isVisible) {
        glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    } else {
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    }

    return glfwCreateWindow(width, height, name, nullptr, share);
}

void OnDestroy() {
    glfwTerminate(); // Error?
    if (window != nullptr) {
        glfwDestroyWindow(window);
    }
}

void OnPreLoad() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void GLFWwindowclosefunCallback(GLFWwindow* window) {
    Settings::IsEngineRunning = false;
}

void GLFWframebuffersizefunCallback(GLFWwindow* window, int width, int height) {
    const u32 scale[2]{static_cast<const u32>(width), static_cast<const u32>(height)};
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
        const char* path = paths[i];

        Engine::HandleEvent(Engine::GR_DROFUNC, (const void*)path);
    }
}

void glfwInstallCallback(Window window) {
    glfwSetWindowCloseCallback(window, GLFWwindowclosefunCallback);

    glfwSetFramebufferSizeCallback(window, GLFWframebuffersizefunCallback);
    GLFWframebuffersizefunCallback(window, SCREEN_WIDTH, SCREEN_HEIGHT);

    glfwSetKeyCallback(window, GLFWkeyfunCallback);

    glfwSetCursorPosCallback(window, GLFWcursorposfunCallback);

    glfwSetScrollCallback(window, GLFWscrollfunCallback);

    glfwSetDropCallback(window, GLFWdropfunCallback);
}

int OnApplicationInit() {
    /* iniicializa o glew */
    if (glfwInit() != GLFW_TRUE) {
        return printf("GLFW do not created");
    }

    OnPreLoad();

    /* cria o window principal */
    window = CreateWindown("GR - GameEngine", SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, true);
    if (window == nullptr) {
        return printf("Window do not created");
    }

    glfwMakeContextCurrent(window);

    glfwInstallCallback(window);

    glfwSetWindowPos(window, 0, 0);

    if (glewInit() != GLEW_OK) {
        OnDestroy();
        return printf("GLEW do not created");
    }

    Input::m_instance = window;

    Engine::HandleEvent(Engine::GR_INITIALIZE, nullptr);

    std::function<void()> func([&]() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    });

    Engine::HandleEvent(Engine::GR_MAINLOOP, &func);

    Engine::HandleEvent(Engine::GR_DESTROY, nullptr);

    OnDestroy();
    return 0;
}

int main() {
    return OnApplicationInit();
}
