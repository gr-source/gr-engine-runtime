#pragma once

#include "gr-engine/application.hpp"

#include "gr-engine/ui/project.hpp"

#include <gr-base/script/default.hpp>

#include <gr-base/core/default.hpp>

class Engine : public Application
{
public:
    enum EngineEvent {
        INITIALIZE     = 1 << 1,
        DESTROY        = 1 << 2,
        RESIZE         = 1 << 3,
        LOOP           = 1 << 7,
        CURSOR_SCROLL  = 1 << 8,
        DROFUNC        = 1 << 9
    };

    Engine();
    ~Engine() override;

    void initialize() override;

    void process() override;

    void shutdown() override;

    void init_menu();

    void init_project(const std::string& path);

    Project &get_project();

    bool has_project() const;

    void DropFile(const std::string &path) override;

private:
    Project m_project;

    bool bHasProject;

    bool check_installed();

    void save_engine_config();

    void update();

    void render();

    void render_gui();
};

