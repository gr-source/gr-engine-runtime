#pragma once

#include <vector>
#include <string>

class File {
public:
    static std::string GetFileContent(const std::string& path);

    static bool Exists(const std::string& path);

    static std::string GetFileName(const std::string &path);

    static std::string GetFile(const std::string &path);

    static bool CreateDirectory(const std::string &path);

    static void Delete(const std::string &path);

    static void Move(const std::string &from, const std::string &to);

    static void Copy(const std::string &from, const std::string &to);
};





