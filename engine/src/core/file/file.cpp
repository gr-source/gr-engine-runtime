#include "file.hpp"

#include <filesystem>
#include <fstream>

std::string File::m_currentPath;

std::string File::m_configPath;

void File::SetCurrentProject(const std::string &path) {
    m_currentPath = path;
}

const std::string &File::GetCurrentProject() {
    return m_currentPath;
}

void File::SetConfigPath(const std::string &path) {
    m_configPath = path;
}

std::string File::GetFileContent(const std::string &path) {
    std::ifstream is(path);
    if (!is.is_open())
        return "";
        
    std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    is.close();
    return content;
}

bool File::Exists(const std::string &path) {
    return std::filesystem::exists(path);
}

std::string File::GetFileName(const std::string &path) {
    auto fileName = std::filesystem::path(path).filename();
    return fileName.replace_extension().string();
}

std::string File::GetFile(const std::string &path, bool internal) {
    return internal ? (m_configPath + "/assets" + path) : (m_currentPath + "/assets" + path);
}

bool File::CreateDirectories(const std::string &path) {
    return std::filesystem::create_directories(path);
}

bool File::CreateDirectory(const std::string &path) {
    return std::filesystem::create_directory(path);
}

void File::Delete(const std::string &path) {
    if (!std::filesystem::exists(path)) {
        return;
    }
    std::filesystem::remove_all(path);
}

void File::Move(const std::string &from, const std::string &to) {
    Copy(from, to);
    Delete(from);
}

void File::Copy(const std::string &from, const std::string &to) {
    if (!std::filesystem::is_directory(from)) {
        auto next = to + "/" + std::filesystem::path(from).filename().string();
        if (std::filesystem::exists(next) && !std::filesystem::is_directory(next)) {
            return;
        }
        return std::filesystem::copy(from, to);
    }
    auto newRoot = to + "/" + std::filesystem::path(from).filename().string();

    CreateDirectory(newRoot);

    for (auto entry : std::filesystem::directory_iterator(from)) {
        auto name = entry.path().filename().string();

        Copy(entry.path(), newRoot);
    }
}

std::string File::getHomeDirectory() {
    auto path = getenv("HOME");
    return std::string(path != nullptr ? path : "/");
}
