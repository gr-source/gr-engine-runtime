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