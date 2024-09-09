#pragma once

#include <vector>
#include <string>
#include <array>

#include "ImGuiRender.h"

struct Project {
    std::string name;
    std::string path;
};

class ProjectMenu {
public:
    static void Initialize();

    static void Shutdown();
    
    static void Render();

private:
    static std::array<const std::string, 6> m_assetsFile;

    static std::vector<Project> m_projects;

    static ImGuiViewport *m_mainViewport;

    static ImGuiStyle *m_style;

    static bool bProjectOpen;

    static ImGuiIO *m_io;

    static void RenderMainMenu();

    static void RenderProject(Project &project);

    static void CreateProject();

    static void CreateProjectInfo(const std::string name, const std::string path);

    static void CreateProjectMetadata(const std::string &path, const std::string &name);

    static void CreateProjectAssets(const std::string &path);

    static void ReadProjects();

    static void OpenProject(Project &project);

    static void RefreshProjectList();

    static void LoadResources();

    static void LoadConfig();

    static void DownloadConfig(const std::string &path);

    static std::string GetCurrentDirProject();
};


