#include "projectMenu.hpp"
#include "editor.hpp"

#include "screenRender.hpp"
#include "renderManager.h"
#include "skybox.h"

#include "core/cmd/cmd.hpp"

#include <gr-base/core/geometry/batching.hpp>

#include <gr-base/core/threadpool.hpp>
#include <gr-base/core/console.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/core/file/file.hpp>

#include "core/file/config.hpp"

#include "scriptManager/scriptManager.hpp"

#include <filesystem>
#include <algorithm>
#include <iostream>

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

std::vector<Project> ProjectMenu::m_projects;

ImGuiStyle *ProjectMenu::m_style = nullptr;

bool ProjectMenu::bInstalling = false;

ImGuiIO *ProjectMenu::m_io = nullptr;

std::shared_ptr<Editor> ProjectMenu::m_editor = nullptr;

void ProjectMenu::Initialize() {
    m_mainViewport = ImGui::GetMainViewport();

    m_style = &ImGui::GetStyle();

    m_io = &ImGui::GetIO();

    auto config = File::getHomeDirectory() + "/.config/gr-engine";
    bInstalling = !File::Exists(config);
    if (bInstalling) {
        InstallConfig(config);
        return;
    }

    InitProject();
}

void ProjectMenu::Shutdown() {
    m_editor = nullptr;

    DebugDraw::Shutdown();
}

void ProjectMenu::Render() {
    if (bInstalling) {
        RenderWindowInstalling();
        return;
    }

    if (Project::GetCurrent()) {
        return m_editor->RenderUI();
    }
    RenderMainMenu();
}

void ProjectMenu::CloseProject() {
    auto path = File::GetFile("/config.ini", true);
    if (File::Exists(path)) {
        File::Delete(path);
    }

    m_editor = nullptr;

    /*Batching::Shutdown();*/

    DebugDraw::Shutdown();

    Scene::SetCurrent(nullptr);

    Project::SetCurrent(nullptr);

    m_projects.clear();

    ReadProjects();
}

Editor *ProjectMenu::GetEditor() {
    return m_editor != nullptr ? &(*m_editor) : nullptr;
}

void ProjectMenu::InitProject() {
    File::SetConfigPath(File::getHomeDirectory() + "/.config/gr-engine");
    File::SetCachePath(File::getHomeDirectory() + "/.cache/gr-engine");
    File::SetUserPath(File::getHomeDirectory() + "/.local/share/gr-engine");

    auto currentProject = GetCurrentDirProject();
    if (!File::Exists(currentProject)) {
        if (!File::CreateDirectories(currentProject)) {
            std::cout << "Falied to create home project directory: " << currentProject << std::endl;
        }
    }

    if (!ReadLastProject()) {
        return ReadProjects();
    }
}

void ProjectMenu::RenderWindowInstalling() {
    ImVec2 vWindowSize(600, 500);

    ImGui::SetNextWindowSize(vWindowSize);
    ImGui::SetNextWindowPos((m_mainViewport->WorkSize - vWindowSize) / 2.0f);
    ImGui::Begin("Install", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    const char *text = "Output:";
    auto textSize = ImGui::CalcTextSize(text);

    ImGui::SetCursorPosX((vWindowSize.x - textSize.x) / 2.0f);
    ImGui::Text("%s", text);
    
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
    ImGui::End();
}

void ProjectMenu::RenderMainMenu() {
    ImGui::SetNextWindowPos({}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(m_mainViewport->Size, ImGuiCond_Always);
    ImGui::Begin("##MainMenu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

    auto bCreateProject = false;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Create")) {
                bCreateProject = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    if (bCreateProject) {
        ImGui::OpenPopup("##create_new_project");
    }

    CreateProject();

    std::for_each(m_projects.begin(), m_projects.end(), &ProjectMenu::RenderProject);

    ImGui::End();
}

void ProjectMenu::RenderProject(Project &project) {
    auto name = project.getName();

    if (ImGui::Selectable(name.c_str())) {
        OpenProject(project);
    }
}

void ProjectMenu::CreateProject() {
    ImVec2 vWindowSize(300.0f, 400.0f);

    ImGui::SetNextWindowSize(vWindowSize);
    ImGui::SetNextWindowPos((m_mainViewport->Size / 2.0f) - (vWindowSize / 2.0f));

    auto isOpen = ImGui::BeginPopupModal("##create_new_project", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (!isOpen) {
        return;
    }

    const char *title = "Create project";

    auto textSize = ImGui::CalcTextSize(title);

    ImGui::SetCursorPosX((vWindowSize.x / 2.0f) - (textSize.x / 2.0f));
    ImGui::Text("%s", title);

    ImGui::Separator();
    ImGui::NewLine();

    ImGui::Text("Name: ");
    ImGui::SameLine();
    
    static char name[64] = {0};
    ImGui::InputText("##create_project_input_name", name, 64);

    ImGui::NewLine();

    ImGui::Text("Path: ");
    ImGui::SameLine();

    std::string path = GetCurrentDirProject() + "/" + name;
    ImGui::InputText("##create_project_input_text_path", path.data(), path.size(), ImGuiInputTextFlags_AlwaysOverwrite);
    
    textSize = ImGui::CalcTextSize("Create");
    ImGui::SetCursorPos({(vWindowSize.x / 2.0f) - (textSize.x / 2.0f), vWindowSize.y - (textSize.y + m_style->WindowPadding.y)});
    if (ImGui::SmallButton("Create")) {
        path.erase(std::remove(path.begin(), path.end(), '\0'), path.end());

        ImGui::CloseCurrentPopup();

        CreateProjectInfo(name, path);
    }

    ImGui::EndPopup();
}

void ProjectMenu::CreateProjectInfo(const std::string name, const std::string path) {
    if (File::Exists(path)) {
        std::cout << "The project: " << name << " exists, in: " << path << std::endl;
        return;
    }
    
    if (!File::CreateDirectory(path)) {
        std::cout << "Unable to create project in directory: " << path << std::endl;
        return;
    }

    CreateProjectMetadata(path, name);
    
    CreateProjectAssets(path);

    RefreshProjectList();
}

void ProjectMenu::CreateProjectMetadata(const std::string &path, const std::string &name) {
    Config config(path + "/metadata.ini");
    config.setValue("name", name);
    config.save();
}

void ProjectMenu::CreateProjectAssets(const std::string &path) {
    for (auto &dir : m_assetsFile) {
        if (dir.empty()) {
            continue;
        }
        auto fullPath = path + "/assets/" + dir;

        if (File::CreateDirectories(fullPath)) {
            continue;
        }
        
        std::cout << "Falied to create directory: " << fullPath << std::endl;
    }

    if (!File::Copy(File::GetAsset("/shader", true), path + "/assets")) {
        Console::LogError("[ CreateProject ] " + File::GetError());
    }
}

void ProjectMenu::ReadProjects() {
    auto path = GetCurrentDirProject();

    if (!File::Exists(path)) {
        return;
    }

    for (auto &entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) {
            continue;
        }

        Config config(entry.path().string() + "/metadata.ini");
        if (!config.is_open()) {
            continue;
        }
        
        config.load();

        if (!config.hasKey("name")) {
            continue;
        }

        m_projects.push_back({config.getValue<std::string>("name"), entry.path().string()});
    }
}

void ProjectMenu::OpenProject(Project &project) {
    File::SetCurrentProject(project.getPath());

    /*Batching::Initialize();*/

    DebugDraw::Initialize();

    Project::SetCurrent(&project);
    
    m_editor = std::make_shared<Editor>();


    SaveLastProject(project);
}

void ProjectMenu::RefreshProjectList() {
    m_projects.clear();

    ReadProjects();
}

void ProjectMenu::InstallConfig(const std::string &config) {
    auto cache = File::getHomeDirectory() + "/.cache/gr-engine";
    auto user = File::getHomeDirectory() + "/.local/share/gr-engine";

    File::SetConfigPath(config);
    File::SetCachePath(cache);
    File::SetUserPath(user);

    Console::LogInfo("Initializing the installation...");

    Console::LogInfo("Creating cache data...");
    File::CreateDirectories(cache);

    Console::LogInfo("Creating user data...");
    File::CreateDirectories(user);

    Console::LogInfo("Installing the base configuration...");
    if (File::CreateDirectories(config)) {
        DownloadConfig(config);
    }
}

void ProjectMenu::DownloadConfig(const std::string &path) {
    Console::LogInfo("Importing rendering data...");
    if (!File::Copy("./assets", path)) {
        Console::LogError("Falied to import data in: %s, to: %s", File::GetError().c_str(), path.c_str());
        return;
    }

    Console::LogInfo("Importing config data...");
    if (!File::Copy("./config", path)) {
        Console::LogError("Falied to import data: " + File::GetError());
        return;
    }

    Console::LogInfo("Importing script data...");
    if (!File::Copy("./script", path)) {
        Console::LogError("Falied to import data: " + File::GetError());
        return;
    }

    Console::LogInfo("Importing component data...");
    if (!File::Copy("./component", path)) {
        Console::LogError("Falied to import data: " + File::GetError());
    }

    Console::LogInfo("Importing example data...");
    if (!File::Copy("./example", path)) {
        Console::LogError("Falied to import data: " + File::GetError());
    }

    ThreadJoin();
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

    auto &project = m_projects.emplace_back(config.getValue<std::string>("name"), path);

    OpenProject(project);
    return true;
}

std::string ProjectMenu::GetCurrentDirProject() {
    return File::getHomeDirectory() + "/gr-project";
}

void ProjectMenu::ThreadJoin() {
    bInstalling = false;

    Console::Shutdown();

    InitProject();
}

// uninstall engine
// rm -rf ~/.cache/gr-engine ~/.config/gr-engine ~/.local/share/gr-engine
