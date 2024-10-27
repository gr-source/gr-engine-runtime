#include "cmd.hpp"

#include "core/console.hpp"

#include <stdio.h>

bool Command::exec(const std::string &cmd) {
    auto file = popen(cmd.c_str(), "r");
    bool success = false;
    if (!file) {
        return success;
    }
    char line[256] = {0};
    while (fgets(line, 256, file)) {
        Console::log(LogInfo::WARNING, line);
    }
    int status = pclose(file);
    if (status == -1) {
        success = false;
    } else {
        success = WIFEXITED(status) && (WEXITSTATUS(status) == 0);
    }
    return success;
}

#include <filesystem>
#include <iostream>
#include <fstream>
bool Command::exec(const std::vector<std::string> &list, std::function<void(std::string)> out) {
    std::string temp_file = std::filesystem::temp_directory_path() / "command.sh";

    std::ofstream os(temp_file);
    for (auto &cmd : list) {
        os << cmd << std::endl;
    }
    os.close();

    auto file = popen(std::string("sh " + temp_file + " 2>&1").c_str(), "r");
    if (!file) {
        return false;
    }

    if (out) {
        char line[1024] = {0};
        while (fgets(line, 1024, file)) {
            out(line);
        }
    }

    pclose(file);

    std::filesystem::remove(temp_file);
    
    return true;
}
