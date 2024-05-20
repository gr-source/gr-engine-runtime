#pragma once

#include <filesystem>

#include "../common.h"

class Filesystem {
public:
    enum : u8 {
        eResource   = 1 << 0,
        eIcon       = 1 << 1,
        eShader     = 1 << 2,
        eScene      = 1 << 3
    };

    static const std::string getpublic(u8 type);

    static std::string getfilecontent(const std::string& path);
};





