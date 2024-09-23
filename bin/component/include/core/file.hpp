#pragma once

#include <fstream>
#include <string>

class File {
public:
    static std::string ReadFile(const std::string &path);

    static bool Exists(const std::string &path);

    static std::string GetFileName(const std::string &path);

    static std::string GetAsset(const std::string &path);

private:
    static std::string GetFile(const std::string &path, bool internal = false);
};




