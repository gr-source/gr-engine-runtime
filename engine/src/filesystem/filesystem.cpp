#include "filesystem.h"

#include "../log.h"

#include <filesystem>

const std::string Filesystem::GetFileContent(const std::string &path) {
    std::ifstream is(path);
    if (!is.is_open())
        return "";
        
    std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    is.close();
    return content;
}

const bool Filesystem::Exists(const std::string &path) {
    return std::filesystem::exists(path);
}

const std::vector<Filesystem::path> Filesystem::getDirectoryEntry(const std::string &path) {
    std::vector<Filesystem::path> entry;
    /*for (auto& it :std::filesystem::directory_iterator(path)) {
        entry.push_back({it.path().parent_path().string(), it.path().string(), it.path().filename()});
    }*/
    return entry;
}

const std::string Filesystem::GetFile(const std::string &path) {
    return "./assets" + path;
}

Filesystem::path::path() {
}

Filesystem::path::path(const std::string &path) : m_path(path) {}

const std::string Filesystem::path::filename() {
    return std::filesystem::path(m_path).filename();
}

std::string Filesystem::path::string() {
    return m_path;
}

const bool Filesystem::path::operator!=(const Filesystem::path &lhs) {
    return (parent != lhs.parent) && (name != lhs.name);
}
