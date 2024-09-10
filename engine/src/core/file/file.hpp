#pragma once

#include <vector>
#include <string>

class File {
public:
    static void SetCurrentProject(const std::string &path);

    static const std::string &GetCurrentProject();

    static void SetConfigPath(const std::string &path);

    static std::string GetFileContent(const std::string &path);

    static bool Exists(const std::string& path);

    static std::string GetFileName(const std::string &path);

    static std::string GetFile(const std::string &path, bool internal = false);

    static std::string GetAsset(const std::string &path, bool internal = false);

    static bool CreateDirectories(const std::string &path);

    static bool CreateDirectory(const std::string &path);

    static void Delete(const std::string &path);

    static void Move(const std::string &from, const std::string &to);

    static void Copy(const std::string &from, const std::string &to);

    static std::string getHomeDirectory();
    
private:
    static std::string m_currentPath;

    static std::string m_configPath;
};
