#pragma once

#include <vector>
#include <string>
#include <atomic>
#include <array>

#include "ImGuiRender.h"
#include "project.hpp"

class ProjectMenu {
public:
    static void Initialize();

    static void Shutdown();
    
    static void Render();

    static void CloseProject();

private:
    static std::array<const std::string, 6> m_assetsFile;

    static std::vector<Project> m_projects;

    static ImGuiViewport *m_mainViewport;

    static std::atomic<bool> bRequestJoin;

    static ImGuiStyle *m_style;

    static bool bInstalling;

    static ImGuiIO *m_io;

    static void InitProject();

    static void RenderWindowInstalling();

    static void RenderMainMenu();

    static void RenderProject(Project &project);

    static void CreateProject();

    static void CreateProjectInfo(const std::string name, const std::string path);

    static void CreateProjectMetadata(const std::string &path, const std::string &name);

    static void CreateProjectAssets(const std::string &path);

    static void ReadProjects();

    static void OpenProject(Project &project);

    static void RefreshProjectList();

    static void InstallConfig(const std::string &path);

    static void DownloadConfig(const std::string &path);

    static void SaveLastProject(Project &project);

    static bool ReadLastProject();

    static std::string GetCurrentDirProject();

    static void InitInstallThread(const std::string &path);

    static void InstallThread(std::string path);

    static void ThreadJoin();
};


