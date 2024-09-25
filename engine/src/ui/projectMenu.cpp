#include "projectMenu.hpp"
#include "editor.hpp"

#include "screenRender.hpp"
#include "renderManager.h"
#include "console.hpp"
#include "skybox.h"

#include "core/cmd/cmd.hpp"

#include "core/threadPool.hpp"
#include "core/scene.hpp"

#include "core/file/initialization.hpp"
#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "scriptManager/scriptManager.hpp"

#include <filesystem>
#include <algorithm>
#include <iostream>

std::array<const std::string, 6> ProjectMenu::m_assetsFile {
    "animation",
    "texture",
    "shader",
    "script",
    "scene",
    "model",
};

ImGuiViewport *ProjectMenu::m_mainViewport = nullptr;

std::atomic<bool> ProjectMenu::bRequestJoin = false;

std::vector<Project> ProjectMenu::m_projects;

ImGuiStyle *ProjectMenu::m_style = nullptr;

bool ProjectMenu::bInstalling = false;

ImGuiIO *ProjectMenu::m_io = nullptr;

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
    Editor::Shutdown();
}

void ProjectMenu::Render() {
    if (bInstalling) {
        RenderWindowInstalling();
        if (bRequestJoin) {
            ThreadJoin();
        }
        return;
    }
    if (Project::GetCurrent()) {
        return Editor::RenderUI();
    }
    RenderMainMenu();
}

void ProjectMenu::CloseProject() {
    auto path = File::GetFile("/config.ini", true);
    if (File::Exists(path)) {
        File::Delete(path);
    }

    Editor::Shutdown();

    Scene::SetCurrent(nullptr);

    Project::SetCurrent(nullptr);

    m_projects.clear();

    ReadProjects();
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
        ReadProjects();
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
        case LogInfo::LogType::INFO:
            color = {0.0f, 1.0f, 0.0f, 1.0f};
            type = "[ INFO ]";
            break;
        case LogInfo::LogType::WARNING:
            color = {1.0f, 1.0f, 0.0f, 1.0f};
            type = "[ WARNING ]";
            break;
        case LogInfo::LogType::ERROR:
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
    auto &name = project.getName();

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
    Initialization ini(path + "/metadata.ini", std::ios_base::out);
    ini.setValue("name", name);
    ini.save();
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
        if (!Stream::open(entry.path().string() + "/metadata.ini", std::ios_base::in)) {
            continue;
        }
        
        std::string line;
        while (std::getline(Stream::GetStream(), line)) {
            auto start = line.find('=');
            if (start == std::string::npos) {
                continue;
            }
            auto key = line.substr(0, start);
            auto value = line.substr(start + 1, line.size());
            if (key != "name") {
                continue;
            }
            m_projects.push_back({value, entry.path().string()});
        }
        Stream::close();
    }
}

void ProjectMenu::OpenProject(Project &project) {
    File::SetCurrentProject(project.getPath());

    Project::SetCurrent(&project);

    Editor::Initialize();

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

    LOG_INFO("Initializing the installation...");

    LOG_INFO("Creating cache data...");
    File::CreateDirectories(cache);

    LOG_INFO("Creating user data...");
    File::CreateDirectories(user);

    LOG_INFO("Installing the base configuration...");
    if (File::CreateDirectories(config)) {
        DownloadConfig(config);
    }
}

void ProjectMenu::DownloadConfig(const std::string &path) {
    LOG_INFO("Importing rendering data...");
    if (!File::Copy("./assets", path)) {
        LOG_ERROR("Falied to import data: " + File::GetError());
        return;
    }

    LOG_INFO("Importing config data...");
    if (!File::Copy("./config", path)) {
        LOG_ERROR("Falied to import data: " + File::GetError());
        return;
    }

    LOG_INFO("Importing script data...");
    if (!File::Copy("./script", path)) {
        LOG_ERROR("Falied to import data: " + File::GetError());
        return;
    }

    LOG_INFO("Importing cmake data...");
    if (!File::Copy("./cmake", path)) {
        LOG_ERROR("Falied to import data: " + File::GetError());
        return;
    }

    LOG_INFO("Importing component data...");
    if (!File::Copy("./component", path)) {
        LOG_ERROR("Falied to import data: " + File::GetError());
        return;
    } else {
        InitInstallThread(path);
    }
}

void ProjectMenu::SaveLastProject(Project &project) {
    Initialization ini(File::GetFile("/config.ini", true), std::ios_base::out);
    if (!ini.is_open()) {
        return;
    }
    ini.setValue("name", project.getName());
    ini.setValue("path", project.getPath());
    ini.save();
}

bool ProjectMenu::ReadLastProject() {
    auto path = File::GetFile("/config.ini", true);
    if (!File::Exists(path)) {
        return false;
    }

    Initialization ini(path, std::ios_base::in);
    if (!ini.is_open()) {
        return false;
    }

    if (ini.empty() || !ini.hasKey("name") || !ini.hasKey("path")) {
        return false;
    }

    path = ini.getValue<std::string>("path");

    if (!File::Exists(path)) {
        return false;
    }

    auto &project = m_projects.emplace_back(ini.getValue<std::string>("name"), path);

    OpenProject(project);
    return true;
}

std::string ProjectMenu::GetCurrentDirProject() {
    return File::getHomeDirectory() + "/gr-project";
}

void ProjectMenu::InitInstallThread(const std::string &path) {
    ThreadPool::AddTask(&ProjectMenu::InstallThread, path);
}

#include <future>
void ProjectMenu::InstallThread(std::string path) {
    LOG_INFO("Initialize build script data...");
    auto component = path + "/component";
    auto script = path + "/script";

    std::vector<std::string> m_list = {
        "export DENGINE_COMPONENT_PATH=" + component,
        "chmod +x " + script + "/configure.sh",
        "sh " + script + "/configure.sh"
    };

    auto func = [](std::string line) {
        LOG_WARNIG(line);
    };

    if (!Command::exec(m_list, func)) {
        LOG_ERROR("Falied to execute command.");
    }

    bRequestJoin = true;
}

void ProjectMenu::ThreadJoin() {
    bRequestJoin = false;
    bInstalling = false;

    Console::Shutdown();

    InitProject();
}

// uninstall engine
// rm -rf ~/.cache/gr-engine ~/.config/gr-engine ~/.local/share/gr-engine ~/gr-project
