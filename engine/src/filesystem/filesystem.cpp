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

const bool Filesystem::CreateDirectory(const std::string &path) {
    return std::filesystem::create_directories(path);
}

void Filesystem::Delete(const std::string &path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

void Filesystem::Move(const std::string &from, const std::string &to) {
    Copy(from, to);
    Delete(from);
}

void Filesystem::Copy(const std::string &from, const std::string &to) {
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
