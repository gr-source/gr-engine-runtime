#include "filesystem.h"

#include "../log.h"

const std::string Filesystem::getpublic(u8 type) {
    switch (type) {
        case eResource:
            return "assets";
        case eIcon:
            return "assets/icon";
        case eShader:
            return "assets/shader";
        case eScene:
            return "assets/scenes";
        default:
            throw std::runtime_error(Log::red + "[ Filesystem::getpublic ] - invalid type." + Log::none);
    }
    return std::string();
}

std::string Filesystem::getfilecontent(const std::string &path) {
    std::ifstream is(path);
    if (!is.is_open()) return "";
    std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    is.close();
    return content;
}

const bool Filesystem::exists(const std::string &path) {
    return std::filesystem::exists(path);
}
