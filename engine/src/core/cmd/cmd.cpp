#include "cmd.hpp"

#include "console.hpp"

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