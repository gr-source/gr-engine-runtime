#pragma once

#include <unordered_map>
#include <string>

class PropertyInspector {
public:
    static void Register(const std::string &className);

    static void find(const std::string &className);
private:
    static std::unordered_map<const std::string, std::tuple<const std::string, int>> s_property;
};


