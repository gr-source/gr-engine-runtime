#include "projectMenu.hpp"
#include "editor.hpp"

#include "engine.h"
#include "imgui.h"
#include "screenRender.hpp"
#include "renderManager.h"
#include "skybox.h"

#include "core/cmd/cmd.hpp"

#include <chrono>
#include <cstring>
#include <gr-base/core/geometry/batching.hpp>

#include <gr-base/core/threadpool.hpp>
#include <gr-base/core/console.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/core/file/file.hpp>

#include "core/file/config.hpp"

#include "scriptManager/scriptManager.hpp"

#include <filesystem>
#include <algorithm>
#include <gr-ini/config.h>
#include <iostream>
#include <thread>

std::array<const std::string, 7> ProjectMenu::m_assetsFile {
    "animation",
    "material",
    "texture",
    "shader",
    "script",
    "scene",
    "model",
};

ImGuiViewport *ProjectMenu::m_mainViewport = nullptr;

ImGuiStyle *ProjectMenu::m_style = nullptr;

ImGuiIO *ProjectMenu::m_io = nullptr;

ProjectMenu::ProjectMenu(Engine *engine) : m_engine(engine), popup_flags(0)
{
    m_mainViewport = ImGui::GetMainViewport();

    m_style = &ImGui::GetStyle();

    m_io = &ImGui::GetIO();

    auto config = File::getHomeDirectory() + "/.config/gr-engine";
    bengine_installed = File::Exists(config);
    if (!bengine_installed)
    {
        Threadpool::AddTask(&ProjectMenu::install_engine, this);

        return;
    }

    setup_project_list();
}

ProjectMenu::~ProjectMenu()
{
    DebugDraw::Shutdown();
}

void ProjectMenu::render()
{
    if (!bengine_installed)
    {
        return create_window_log();
    }

    create_window_project_list();

    switch (popup_flags)
    {
        case ProjectMenuPopup_CreateProject:
            ImGui::OpenPopup("##popup_create_new_project");
            break;
        default:
            break;
    }

    popup_create_new_project();

    popup_flags = 0;
}

void ProjectMenu::close_project()
{
    auto path = File::GetFile("/config.ini", true);
    if (File::Exists(path)) {
        File::Delete(path);
    }

    /*Batching::Shutdown();*/

    DebugDraw::Shutdown();

    Scene::SetCurrent(nullptr);

    Project::SetCurrent(nullptr);

    m_projects.clear();

    read_project_directory();
}

// ********** private ********** //
void ProjectMenu::create_window_log()
{
    ImVec2 window_size = {600.0f, 500.0f};

    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    ImGui::SetNextWindowPos((m_mainViewport->WorkSize - window_size) / 2.0f);
    if (ImGui::Begin("Install", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
    {
        render_log();
    }

    ImGui::End();
}

void ProjectMenu::render_log()
{
    // const char *text = "Output:";
    // auto textSize = ImGui::CalcTextSize(text);

    // ImGui::SetCursorPosX((window_size.x - textSize.x) / 2.0f);
    // ImGui::Text("%s", text);
    
    for (auto &log : Console::GetListLogs()) {
        ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        std::string type = "[ DEFAULT ]";

        switch (log.type) {
        case Log::LogType::INFO:
            color = {0.0f, 1.0f, 0.0f, 1.0f};
            type = "[ INFO ]";
            break;
        case Log::LogType::WARNING:
            color = {1.0f, 1.0f, 0.0f, 1.0f};
            type = "[ WARNING ]";
            break;
        case Log::LogType::ERROR:
            color = {1.0f, 0.0f, 0.0f, 1.0f};
            type = "[ ERROR ]";
            break;
        default:
            break;
        }

        ImGui::TextColored(color, "%s", type.c_str());
        ImGui::SameLine();
        ImGui::Text("%s", log.message.c_str());

    }

    if (Console::bChanged) {
        ImGui::SetScrollHereY(1.0f);
        Console::bChanged = false;
    }
}

void ProjectMenu::create_window_project_list()
{
    ImGui::SetNextWindowPos(ImVec2{0.0f, 0.0f}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(m_mainViewport->Size, ImGuiCond_Always);
    if (ImGui::Begin("##create_window_project_list", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
    {
        render_project_list();
    }

    ImGui::End();
}

void ProjectMenu::render_project_list()
{
    auto avail = ImGui::GetContentRegionAvail();
    if (ImGui::Button("Create New Project", ImVec2{avail.x, 0.0f}))
    {
        popup_flags |= ProjectMenuPopup_CreateProject;
    }

    for (auto &project : m_projects)
    {
        if (ImGui::Selectable(project.getName().c_str()))
        {
            m_engine->set_project(project);
        }
    }
}

void ProjectMenu::setup_project_list()
{

    // if (!ReadLastProject())
    {
        return read_project_directory();
    }
}

void ProjectMenu::popup_create_new_project()
{
    ImVec2 window_size = {300.0f, 400.0f};

    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    ImGui::SetNextWindowPos((m_mainViewport->Size / 2.0f) - (window_size / 2.0f));

    auto isOpen = ImGui::BeginPopupModal("##popup_create_new_project", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (!isOpen)
    {
        return;
    }

    ImGui::SeparatorText("Create Project");

    ImGui::NewLine();

    ImGui::Text("Name: ");
    ImGui::SameLine();
    
    static char name[64] = {0};
    ImGui::InputText("##create_project_input_name", name, 64);

    ImGui::NewLine();

    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", ImVec2{avail.x, 0.0f}))
    {
        create_project(name, gfile::get_home_directory() + "/gr-engine/" + name);

        std::memset(name, 0, sizeof(name));

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void ProjectMenu::create_project(const std::string name, const std::string path)
{
    if (gfile::exists(path))
    {
        std::cout << "The project: " << name << " exists, in: " << path << std::endl;
        return;
    }
    
    if (!gfile::create_directory(path))
    {
        std::cout << "Unable to create project in directory: " << path << std::endl;
        return;
    }

    if (!create_project_metadata(path, name))
    {
        gfile::remove(path);

        std::cout << "[ Project ] Fail to create project metadada." << std::endl;
        return;
    }
    
    CreateProjectAssets(path);

    m_projects.emplace_back(name, path);
}

bool ProjectMenu::create_project_metadata(const std::string &path, const std::string &name)
{
    auto config = config_create();
    if (config == nullptr)
    {
        return false;
    }

    {
        auto section = config_create_section(config, "project", nullptr);
        config_set_string(section, "name", name.c_str());
    }

    config_save((path + "/metadata.ini").c_str(), config);

    config_release(config);

    return true;
}

void ProjectMenu::CreateProjectAssets(const std::string &path)
{
    if (!gfile::exists(path + "/assets"))
    {
        gfile::create_directory(path + "/assets");
    }

    for (auto &dir : m_assetsFile)
    {
        auto fullPath = path + "/assets/" + dir;

        if (!gfile::exists(fullPath))
        {
            gfile::create_directory(fullPath);
        }
    }

    if (!File::Copy(File::GetAsset("/shader", true), path + "/assets"))
    {
        Console::LogError("[ CreateProject ] " + File::GetError());
    }
}

void ProjectMenu::read_project_directory()
{
    auto dir = gfile::get_home_directory() + "/gr-engine";
    if (!gfile::exists(dir))
    {
        return;
    }

    for (auto &entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_directory() || !gfile::exists(entry.path() / "metadata.ini"))
        {
            continue;
        }

        auto config = config_create();
        if (config == nullptr)
        {
            continue;
        }

        config_load((entry.path() / "metadata.ini").c_str(), config);
        {
            auto section = config_find_section(config, "project");
            if (section != nullptr)
            {
                auto name = config_get_string(section, "name");

                m_projects.emplace_back(name, entry.path().string());
            }
        }
        config_release(config);
    }
}

void ProjectMenu::OpenProject(Project &project) {
    File::SetCurrentProject(project.getPath());

    /*Batching::Initialize();*/

    DebugDraw::Initialize();

    Project::SetCurrent(&project);
    
    SaveLastProject(project);
}

void ProjectMenu::install_engine()
{
    Console::LogInfo("[ Installer ] iniciando instalação...");

    if (!gfile::exists(gfile::get_home_directory() + "/gr-engine"))
    {
        gfile::create_directory(gfile::get_home_directory() + "/gr-engine");
    }

    if (!gfile::exists(gfile::get_data_directory()))
    {
        gfile::create_directory(gfile::get_data_directory());
    }

    if (!gfile::exists(gfile::get_config_directory()))
    {
        gfile::create_directory(gfile::get_config_directory());
    }

    if (!gfile::exists(gfile::get_cache_directory()))
    {
        gfile::create_directory(gfile::get_cache_directory());
    }

    Console::LogInfo("[ Installer ] copiando arquivos...");

    if (!download_data())
    {
        gfile::remove(gfile::get_data_directory());
        gfile::remove(gfile::get_config_directory());
        gfile::remove(gfile::get_cache_directory());
        return;
    }

    Console::LogInfo("[ Installer ] arquivos copiados com sucesso.");

    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    stop_install();
}

bool ProjectMenu::download_data()
{
    std::this_thread::sleep_for(std::chrono::seconds(3));

    Console::LogInfo("[ Installer ] copiando assets...");
    if (!File::Copy("./assets", gfile::get_data_directory()))
    {
        Console::LogError("[ Installer ] fail to copy assets.");
        return false;
    }

    Console::LogInfo("[ Installer ] copiando arquivos de config...");
    if (!File::Copy("./config", gfile::get_data_directory()))
    {
        Console::LogError("[ Installer ] fail to copy config.");
        return false;
    }

    Console::LogInfo("[ Installer ] copiando scripts...");
    if (!File::Copy("./script", gfile::get_data_directory()))
    {
        Console::LogError("[ Installer ] fail to copy script.");
        return false;
    }

    Console::LogInfo("[ Installer ] copiando compiler...");
    if (!File::Copy("./component", gfile::get_data_directory()))
    {
        Console::LogError("[ Installer ] fail to copy component.");
        return false;
    }

    Console::LogInfo("[ Installer ] copiando arquivos de exemplo...");
    if (!File::Copy("./example", gfile::get_data_directory()))
    {
        Console::LogError("[ Installer ] fail to copy examples");
        return false;
    }

    return true;
}

void ProjectMenu::SaveLastProject(Project &project) {
    Config config(File::GetFile("/config.ini", true));
    config.setValue("name", project.getName());
    config.setValue("path", project.getPath());
    config.save();
}

bool ProjectMenu::ReadLastProject() {
    auto path = File::GetFile("/config.ini", true);
    if (!File::Exists(path)) {
        return false;
    }

    Config config(path);
    if (!config.is_open()) {
        return false;
    }

    config.load();

    if (!config.hasKey("name") || !config.hasKey("path")) {
        return false;
    }

    path = config.getValue<std::string>("path");

    if (!File::Exists(path)) {
        return false;
    }

    // auto &project = m_projects.emplace_back(config.getValue<std::string>("name"), path);

    // OpenProject(project);
    return true;
}

void ProjectMenu::stop_install()
{
    Console::Shutdown();

    setup_project_list();

    bengine_installed = true;
}

// uninstall engine
// rm -rf ~/.cache/gr-engine ~/.config/gr-engine ~/.local/share/gr-engine
