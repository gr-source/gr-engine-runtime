#include <engine.h>
#include <settings.h>

#include <GLFW/glfw3.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <core/stb/stb_image.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 720


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
    engine::OnResizeWindow(u32(width), u32(height));
}

void GLFWkeyfunCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        engine::Escap();
    }
}

void glfwInstallCallback(Window window) {
    glfwSetFramebufferSizeCallback(window, GLFWframebuffersizefunCallback);
    GLFWframebuffersizefunCallback(window, SCREEN_WIDTH, SCREEN_HEIGHT);

    glfwSetKeyCallback(window, GLFWkeyfunCallback);

    glfwSetWindowCloseCallback(window, GLFWwindowclosefunCallback);
}

int OnApplicationInit() {
    /* iniicializa o glew */
    if (glfwInit() != GLFW_TRUE) {
        return printf("GLFW do not created");
    }

    OnPreLoad();

    /* cria o window principal */
    window = CreateWindown("GR - GameRender", SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, true);
    if (window == nullptr) {
        return printf("Window do not created");
    }

    glfwInstallCallback(window);

    glfwMakeContextCurrent(window);

    glfwSetWindowPos(window, 0, 0);

    if (glewInit() != GLEW_OK) {
        OnDestroy();
        return printf("GLEW do not created");
    }

    engine::WindowInstance((void*)window);

    Settings::IsEngineRunning = true;

    engine::OnInit();

    engine::MainLoop([&]{
        glfwSwapBuffers(window);
        glfwPollEvents();
    });
    
    Settings::IsEngineRunning = false;

    engine::OnDestroy();

    OnDestroy();
    return 0;
}

int main() {
    return OnApplicationInit();
}
