#pragma once

#include <unordered_map>
#include <string>

class NameManager {
public:
    static void Shutdown();

    static std::string GenerateName(std::string name);

private:
    static std::unordered_map<std::string, int> m_nameID;

    static std::size_t FindDelimiter(char delimiter, const std::string &name);
};


