#include "project.hpp"

extern "C"
{
#include <gr-ini/config.h>
}

#include <gr-base/core/file/file.hpp>

Project *Project::m_current = nullptr;

void Project::SetCurrent(Project *project) {
    m_current = project;
}

Project *Project::GetCurrent() {
    return m_current;
}

Project::Project(const std::string &name, const std::string &path) : m_name(name), m_path(path) {}

std::string Project::getPath() const {
    return m_path;
}

std::string Project::getName() const {
    return m_name;
}

void Project::save()
{
    auto config = config_create();
    if (config == nullptr)
        return;

    {
        auto section = config_create_section(config, "project", nullptr);
        config_set_string(section, "name", m_name.c_str());
        config_set_string(section, "path", m_path.c_str());
    }

    config_save(config, (gfile::get_config_directory() + "/config.ini").c_str());

    config_release(config);
}

bool Project::load()
{
    return true;
}


