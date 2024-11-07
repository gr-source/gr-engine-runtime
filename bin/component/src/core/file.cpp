#include "core/file.hpp"

#include "core/function.hpp"

#include <filesystem>
#include <fstream>

std::string File::ReadFile(const std::string & path) {
    std::ifstream is(path);
    if (!is.is_open()) {
        return "";
    }
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

std::string File::GetAsset(const std::string &path) {
    return GetFile("/assets/" + path, false);
}

std::string (*File::GetFile)(const std::string &path, bool internal);
INSERT_FUNCTION(0, File::GetFile);
