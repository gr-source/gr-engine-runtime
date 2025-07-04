#include "project.hpp"

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

