#include "projectMenu.hpp"
#include "editor.hpp"

#include "screenRender.hpp"
#include "renderManager.h"
#include "console.hpp"
#include "skybox.h"

#include "core/file/initialization.hpp"
#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/scene.hpp"

#include "scriptManager/scriptManager.hpp"

#include <filesystem>
#include <algorithm>
#include <iostream>

std::array<const std::string, 6> ProjectMenu::m_assetsFile {
    "animation",
    "texture",
    "shader",
    "scene",
    "model",
    "build",
};

ImGuiViewport *ProjectMenu::m_mainViewport = nullptr;

std::vector<Project> ProjectMenu::m_projects;

ImGuiStyle *ProjectMenu::m_style = nullptr;

ImGuiIO *ProjectMenu::m_io = nullptr;

void ProjectMenu::Initialize() {
    m_mainViewport = ImGui::GetMainViewport();

    m_style = &ImGui::GetStyle();

    m_io = &ImGui::GetIO();

    LoadConfig();

    auto currentProject = GetCurrentDirProject();
    if (!File::Exists(currentProject)) {
        if (!File::CreateDirectories(currentProject)) {
            std::cout << "Falied to create home project directory: " << currentProject << std::endl;
        }
    }

    LoadResources();

    if (!ReadLastProject()) {
        ReadProjects();
    }
}

void ProjectMenu::Shutdown() {
    Editor::Shutdown();
}

void ProjectMenu::Render() {
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
    ImVec2 size(300.0f, 400.0f);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos((m_mainViewport->Size / 2.0f) - (size / 2.0f));

    auto isOpen = ImGui::BeginPopupModal("##create_new_project", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (!isOpen) {
        return;
    }

    ImGui::SetCursorPosX((size.x / 2.0f) - (ImGui::CalcTextSize("Create project").x / 2.0f));
    ImGui::Text("Create project");

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
    ImGui::InputText("##create_project_input_path", path.data(), path.size(), ImGuiInputTextFlags_AlwaysOverwrite);
    
    auto textSize = ImGui::CalcTextSize("Create");
    ImGui::SetCursorPos({(size.x / 2.0f) - (textSize.x / 2.0f), size.y - (textSize.y + m_style->WindowPadding.y)});
    if (ImGui::SmallButton("Create")) {
        path.erase(std::remove(path.begin(), path.end(), '\0'), path.end());

        CreateProjectInfo(name, path);
        ImGui::CloseCurrentPopup();
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

void ProjectMenu::LoadResources() {
    Editor::LoadReources();
}

void ProjectMenu::LoadConfig() {
    auto config = File::getHomeDirectory() + "/.config/gr-engine";
    auto cache = File::getHomeDirectory() + "/.cache/gr-engine";

    File::SetConfigPath(config);
    File::SetCachePath(cache);

    if (File::CreateDirectories(cache)) {}

    if (File::CreateDirectories(config)) {
        DownloadConfig(config);
    }
}

void ProjectMenu::DownloadConfig(const std::string &path) {
    File::Copy("./assets", path);
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
