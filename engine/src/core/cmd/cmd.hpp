#pragma once

#include <functional>
#include <vector>
#include <string>

class Command {
public:
    static bool exec(const std::string &cmd);

    static bool exec(const std::vector<std::string> &list, std::function<void(std::string)> out = nullptr);
};




