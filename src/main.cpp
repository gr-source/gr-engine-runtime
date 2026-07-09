#include "gr-base/core/file/file.hpp"

#include <gr-math/gmath.hpp>

#include <gr-render/gRender.h>

#include "gr-engine/timer.hpp"

#include <gr-base/module/moduleloader.hpp>

#include <gr-base/application.hpp>

#include <gr-base/core/logger.hpp>
#include <gr-base/core/window.hpp>
#include <gr-base/core/input.hpp>
#include <gr-base/core/clock.hpp>

#include <string.h>

#include <GLFW/glfw3.h>

extern "C"
{
#include <gr-ini/config.h>
}

// #define ENABLE_VSYNC

GLFWwindow *CreateWindow(const char* name, int width, int height, GLFWwindow * share, bool isVisible)
{
    glfwWindowHint(GLFW_VISIBLE, isVisible ? GL_TRUE : GL_FALSE);

    return glfwCreateWindow(width, height, name, nullptr, share);
}

void GLFWframebuffersizefunCallback(GLFWwindow* window, int width, int height)
{
}

static Vector2 current_scroll_offset = type_traits<Vector2>::zero;
static Vector2 mouse_scroll = type_traits<Vector2>::zero;

void GLFWscrollfunCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    current_scroll_offset.x += static_cast<float>(xoffset);
    current_scroll_offset.y += static_cast<float>(yoffset);
}

void GLFWdropfunCallback(GLFWwindow* window, int path_count, const char* paths[])
{
    Application* app = Application::application;
    if (app == nullptr)
        return;

    std::vector<std::string> files;
    for (int i=0;i<path_count;i++)
        files.push_back(paths[i]);

    app->dropPath(files);
}

void glfwInstallCallback(GLFWwindow * window) {
    glfwSetFramebufferSizeCallback(window, GLFWframebuffersizefunCallback);

    glfwSetScrollCallback(window, GLFWscrollfunCallback);

    glfwSetDropCallback(window, GLFWdropfunCallback);
}

void MyGLFWerrorfun(int error, const char* description)
{
    logger::error("Erro interno do glfw '%s' code '%d'.", description, error);
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

static Vector2 last_mouse_pos = type_traits<Vector2>::zero;
static Vector2 mouse_delta = type_traits<Vector2>::zero;

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
    current_scroll_offset = type_traits<Vector2>::zero;
}

struct app_settings
{
    bool vsync;
} settings;

static void app_set_vsync(bool value)
{
    // if (settings.vsync == value)
        // return;

    glfwSwapInterval(value ? GLFW_TRUE : GLFW_FALSE);

    logger::info("[Main] VSync %s.", (value ? "enable" : "disable"));

    settings.vsync = value;
}

static bool app_get_vsync()
{
    return settings.vsync;
}

static void config_set_engine_callback()
{
    // ********** Window ********** //
    Window::SetWindowTitle  = &glfw_set_window_title;

    Window::SetWindowSize   = &glfw_set_window_size;

    Window::GetWindowSize   = &glfw_get_window_size;

    Window::SetVSync        = &app_set_vsync;
    Window::GetEnableVSync  = &app_get_vsync;

    // ********** Input ********** //
    Input::IsKeyPress       = &glfw_is_key_press;
    
    Input::SetCursorPos     = &glfw_set_cursor_pos;
    Input::GetCursorPos     = &glfw_get_cursor_pos;

    Input::IsMousePress     = &glfw_get_mouse_button;

    Input::GetMouseDelta    = &glfw_get_mouse_delta;
    Input::GetMouseScroll   = &glfw_get_mouse_scroll;
}

#include <sys/signal.h>

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
    printf("Caught segfault at address %p\n", si->si_addr);

    // destroy();
    
    exit(1);
}

static void app_set_default_window_config()
{
    Window::SetWindowSize({1024.0f, 600.0f});

    Window::SetVSync(false);
}

static void app_set_config()
{
    std::string dir = gfile::get_config_directory() + "/app.ini";

    if (!gfile::exists(dir))
        return app_set_default_window_config();

    gConfig* config = config_create();
    if (config == NULL)
        return;

    config_load(config, dir.c_str());

    // window
    {
        gSection* section = config_find_section(config, "window");
        if (section != NULL)
        {
            float width = config_get_float(section, "width");
            float height = config_get_float(section, "height");

            Window::SetWindowSize({width, height});
        }
    }

    // application
    {
        gSection* section = config_find_section(config, "application");
        if (section != NULL)
        {
            Window::SetVSync(config_get_bool(section, "vsync"));
            
            Clock::max_fps = static_cast<double>(config_get_int(section, "max_fps"));
        }
    }

    config_release(config);
}

static void app_save_config()
{
    Vector2 window_size = Window::GetWindowSize();

    std::string dir = gfile::get_config_directory() + "/app.ini";

    gConfig* config = config_create();
    if (config == NULL)
        return;

    {
        gSection* section = config_create_section(config, "window");
        if (section != NULL)
        {
            config_set_float(section, "width", window_size.x);
            config_set_float(section, "height", window_size.y);
        }
    }

    {
        gSection* section = config_create_section(config, "application");
        if (section != NULL)
        {
            config_set_bool(section, "vsync", Window::GetEnableVSync());
            config_set_int(section, "max_fps", Clock::max_fps);
        }
    }

    config_save(config, dir.c_str());

    config_release(config);
}

static void SetupAppDirectories()
{
    if (!gfile::exists(gfile::get_cache_directory()))
        gfile::create_directory(gfile::get_cache_directory());

    if (!gfile::exists(gfile::get_data_directory()))
        gfile::create_directory(gfile::get_data_directory());

    if (!gfile::exists(gfile::get_config_directory()))
        gfile::create_directory(gfile::get_config_directory());

    if (!gfile::exists(gfile::get_home_directory() + "/gr-engine"))
        gfile::create_directory(gfile::get_home_directory() + "/gr-engine");
}

static void Run(Application* app)
{
    logger::info("[ Main ] Setup window.");

    if (glfwInit() != GLFW_TRUE)
        return logger::critical("[ Main ] Falha ao iniciar o GLFW.");

    glfwSetErrorCallback(MyGLFWerrorfun);

    logger::debug("[Main] Usding glfw version '%s'.", glfwGetVersionString());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    logger::debug("[Main] Versão a ser usada do opengl 3.3");

    {
        switch (glfwGetPlatform())
        {
            case GLFW_PLATFORM_WAYLAND:
                logger::debug("[Main] Plataforma: Wayland");
                break;
            case GLFW_PLATFORM_X11:
                logger::debug("[Main] Plataforma: X11");
                break;
            case GLFW_PLATFORM_WIN32:
                logger::debug("[Main] Plataforma: Windows");
                break;
            case GLFW_PLATFORM_COCOA:
                logger::debug("[Main] Plataforma: macOS");
                break;
            case GLFW_PLATFORM_NULL:
                logger::debug("[Main] Plataforma: Null (simulação)");
                break;
            default:
                logger::debug("[Main] Plataforma desconhecida");
                break;
        }   
    }

    {
        int count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);

        logger::debug("[Main] Número de monitores: %d.", count);

        for (int i = 0; i < count; ++i)
        {
            const char* name = glfwGetMonitorName(monitors[i]);

            logger::debug("[Main] Monitor %d: %s", i, (name != nullptr ? name : "Nome não disponível"));
        }
    }

    GLFWwindow* window = CreateWindow("GR - GameEngine", 1024, 512, nullptr, true);
    if (window == nullptr)
    {
        glfwTerminate();

        return logger::critical("[ Main ] Falha ao criar a uma janela.");
    }

    glfwMakeContextCurrent(window);

    glfwInstallCallback(window);

    if (!gr::gRender::Initialize())
    {
        glfwDestroyWindow(window);

        glfwTerminate();

        return logger::critical("[Main] Falha ao iniciar o renderizador.");
    }

    config_set_engine_callback();

    app_set_config();

    Timer time(Clock::max_fps);

    logger::info("[ Main ] Application '0x%lX' initialize.", app);
    app->initialize();

    // run
    while (!glfwWindowShouldClose(window))
    {
        // input 
        glfwPollEvents();

        update_input_states();

        // frame calculate
        time.begin_frame();

        Clock::deltaTime = time.get_delta();
        Clock::time += Clock::deltaTime;

        app->run();

        glfwSwapBuffers(window);

        time.end_frame();
    }

    logger::info("[ Main ] Application '0x%lX'shutdown.", app);
    app->shutdown();

    // Shutdown
    app_save_config();

    glfwDestroyWindow(window);

    glfwTerminate();
}

static void SetupModule()
{
    try
    {
        ModuleLoader::Load("editor", "/home/NoirVoid/Projetos/gr-engine-dev/modules/gr-engine-editor/lib/libgr-engine-editor.so");
    } catch (const std::exception& e)
    {
        return logger::critical("[ Main ] Falha ao carregar o modulo erro='%s'", e.what());
    }

    auto CreateInstance = ModuleLoader::GetFunction<Application*>("editor", "CreateInstance");
    if (CreateInstance == nullptr)
        return logger::critical("[ Main ] Símbolo 'CreateInstance' não encontrado no módulo 'editor'.");

    Application* app = CreateInstance();

    Run(app);

    delete app;
}

int main(int argc, char *argv[])
{
    SetupAppDirectories();

    logger_config config;
    config.path = gfile::get_config_directory() + "/logs";

    logger::Initialize(&config);

    logger::info("[ Main ] Initialize.");

    ModuleLoader::Initialize();

    SetupModule();

    ModuleLoader::Shutdown();

    logger::info("[ Main ] Shutdown.");

    logger::Shutdown();
}




