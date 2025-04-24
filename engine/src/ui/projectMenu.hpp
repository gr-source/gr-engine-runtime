#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <atomic>
#include <array>
#include <memory>

#include "ImGuiRender.h"
#include "project.hpp"

#include "viewlayout.hpp"

class Editor;
class Engine;

using ProjectMenuPopup_ = std::uint32_t;

enum ProjectMenuPopup : ProjectMenuPopup_
{
    ProjectMenuPopup_CreateProject = 1 << 1,
};

class ProjectMenu : public ViewLayout
{
public:
    ProjectMenu(Engine *engine);
    ~ProjectMenu() override;
    
    void onRender() override;

    void close_project();

private:
    static std::array<const std::string, 7> m_assetsFile;

    std::vector<Project> m_projects;

    Engine *m_engine;

    static ImGuiViewport *m_mainViewport;

    static ImGuiStyle *m_style;

    bool bengine_installed;

    ProjectMenuPopup_ popup_flags;

    static ImGuiIO *m_io;

    void create_window_log();

    void render_log();

    void create_window_project_list();

    void render_project_list();

    void setup_project_list();

    void popup_create_new_project();

    void create_project(const std::string name, const std::string path);

    bool create_project_metadata(const std::string &path, const std::string &name);

    static void CreateProjectAssets(const std::string &path);

    void read_project_directory();

    static void OpenProject(Project &project);

    void install_engine();

    bool download_data();

    static void SaveLastProject(Project &project);

    static bool ReadLastProject();

    void stop_install();
};


