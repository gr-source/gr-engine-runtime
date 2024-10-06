#include <settings.h>
#include <engine.h>

#include <linux/input-event-codes.h>

#include <GLFW/glfw3.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600

std::unordered_map<int, grm::u32> m_keyMap = {
    {GLFW_KEY_ESCAPE, KEY_ESC},
    {GLFW_KEY_W, KEY_W},
    {GLFW_KEY_S, KEY_S},
    {GLFW_KEY_A, KEY_A},
    {GLFW_KEY_D, KEY_D}
};

typedef GLFWwindow *Window;

Window window = nullptr;

Window CreateWindown(const char* name, int width, int height, Window share, bool isVisible) {
    glfwWindowHint(GLFW_VISIBLE, isVisible ? GL_TRUE : GL_FALSE);

    return glfwCreateWindow(width, height, name, nullptr, share);
}

void OnPreLoad() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_SAMPLES, 4);
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
        const char* path = paths[i];

        Engine::HandleEvent(Engine::GR_DROFUNC, (const void*)path);
    }
}

void glfwInstallCallback(Window window) {
    glfwSetFramebufferSizeCallback(window, GLFWframebuffersizefunCallback);

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

    glfwSwapInterval(0);

    glfwInstallCallback(window);

    glfwSetWindowPos(window, 0, 0);

    if (!grr::gRender::Initialize()) {
        glfwTerminate(); // Error?
        if (window != nullptr) {
            glfwDestroyWindow(window);
        }
        return printf("GLEW do not created");
    }

    Settings::StartPlayEngine();

    Engine::HandleEvent(Engine::GR_INITIALIZE, nullptr);

    GLFWframebuffersizefunCallback(window, SCREEN_WIDTH, SCREEN_HEIGHT);

    while (Settings::IsEngineRunning() && !glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();

        Engine::HandleEvent(Engine::GR_IDLE, nullptr);
    }
    
    Settings::StopPlayEngine();

    Engine::HandleEvent(Engine::GR_DESTROY, nullptr);

    glfwTerminate(); // Error?
    if (window != nullptr) {
        glfwDestroyWindow(window);
    }

    return 0;
}

int main() {
    return OnApplicationInit();
}




