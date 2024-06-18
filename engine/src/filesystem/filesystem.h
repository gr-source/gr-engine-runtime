#pragma once

#include <vector>

#include "../common.h"

class Filesystem {
public:
    enum : u8 {
        eResource   = 1 << 0,
        eIcon       = 1 << 1,
        eShader     = 1 << 2,
        eScene      = 1 << 3,
        eModel      = 1 << 4
    };

    struct path {
        std::string m_path;
        std::string parent;
        std::string name;

        path();

        path(const std::string& path);

        const std::string filename();

        std::string string();

        const bool operator!=(const Filesystem::path& lhs);
    };

    static Filesystem::path getpublic(u8 type);

    static std::string getfilecontent(const std::string& path);

    static const bool exists(const std::string& path);

    static const std::vector<Filesystem::path> getDirectoryEntry(const std::string& path);

    // static Filesystem::path path(const std::string);
};





