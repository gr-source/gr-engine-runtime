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

void GLFWerrorfunc(int error_code, const char* description) {
    // std::cout << "GLFW ERROR: " << error_code << " " << description << std::endl;
}

void glfwInstallCallback(Window window) {
    engine::OnScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    glfwSetKeyCallback(window, engine::GLFWkeyfunCallback);

    glfwSetFramebufferSizeCallback(window, engine::GLFWframebuffersizefunCallback);

    glfwSetDropCallback(window, engine::GLFWdropfun);

    glfwSetWindowCloseCallback(window, GLFWwindowclosefunCallback);

    glfwSetErrorCallback(GLFWerrorfunc);
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

    Settings::IsEngineRunning = true;

    engine::OnInit();

    std::thread th_loader(engine::OnLoader);

    std::thread th_physics(engine::OnPhysics);

    while (Settings::IsEngineRunning || !glfwWindowShouldClose(window)) {
        engine::OnUpdate();

        engine::OnRender();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    Settings::IsEngineRunning = false;

    if (th_loader.joinable()) {
        th_loader.join();
    }

    if (th_physics.joinable()) {
        th_physics.join();
    }

    engine::OnDestroy();

    OnDestroy();
    return 0;
}

int main() {
    return OnApplicationInit();
}
