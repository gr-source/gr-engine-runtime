#pragma once

#include <vector>

#include "../common.h"

#include <gr-math/gmath.h>

class Filesystem {
public:
    enum : grm::u8 {
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

    static const std::string GetFileContent(const std::string& path);

    static const bool Exists(const std::string& path);

    static const std::vector<Filesystem::path> getDirectoryEntry(const std::string& path);

    static const std::string GetFileName(const std::string &path);

    static const std::string GetFile(const std::string &path);

    static const bool CreateDirectory(const std::string &path);

    static void Delete(const std::string &path);

    static void Move(const std::string &from, const std::string &to);

    static void Copy(const std::string &from, const std::string &to);
};





