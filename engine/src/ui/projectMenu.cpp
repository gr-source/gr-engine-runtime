#include "projectMenu.hpp"

#include "screenRender.hpp"
#include "renderManager.h"
#include "console.hpp"
#include "skybox.h"
#include "Game.h"

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
    "scene",
    "model",
    "build",
};

ImGuiViewport *ProjectMenu::m_mainViewport = nullptr;

std::vector<Project> ProjectMenu::m_projects;

ImGuiStyle *ProjectMenu::m_style = nullptr;

bool ProjectMenu::bProjectOpen = false;

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

    ReadProjects();
}

void ProjectMenu::Shutdown() {
    if (!bProjectOpen) {
        return;
    }

    RenderManager::Shutdown();

    Skybox::Shutdown();

    Game::Shutdown();

    ScreenRender::Shutdown();

    ScriptManager::Shutdown();
}

void ProjectMenu::Render() {
    if (bProjectOpen) {
        return Game::RenderUI();
    }
    RenderMainMenu();
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
    auto &name = project.name;

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
    if (!Stream::open(path + "/metadata.bin", std::ios_base::out)) {
        return;
    }
    Stream::GetStream() << "name=" << name << std::endl;
    Stream::close();
}

void ProjectMenu::CreateProjectAssets(const std::string &path) {
    for (auto &dir : m_assetsFile) {
        auto fullPath = path + "/assets/" + dir;
        

        // if (File::Exists(fullPath)) {
        //     continue;
        // }

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
        if (!Stream::open(entry.path().string() + "/metadata.bin", std::ios_base::in)) {
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
    File::SetCurrentProject(project.path);

    ScriptManager::Initialize("/home/grsource/Documentos/AppProjects/gr-component/lib/libgr-component.so");

    RenderManager::Initialize();

    ScreenRender::Initialize();

    Skybox::Initialize();

    Game::Initialize();

    bProjectOpen = true;
}

void ProjectMenu::RefreshProjectList() {
    m_projects.clear();

    ReadProjects();
}

void ProjectMenu::LoadResources() {
    auto lastPath = File::GetCurrentProject();

    File::SetCurrentProject("/home/grsource/Documentos/AppProjects/gr-engine/bin/");

    Game::LoadReources();

    File::SetCurrentProject(lastPath);
}

void ProjectMenu::LoadConfig() {
    auto config = File::getHomeDirectory() + "/.config/gr-engine";

    File::CreateDirectories(config);
}

std::string ProjectMenu::GetCurrentDirProject() {
    return File::getHomeDirectory() + "/gr-project";
}
