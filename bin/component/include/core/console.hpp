#pragma once

#include <sstream>
#include <string>

struct LogInfo {
    enum LogType {
        DEFAULT = 0,
        ERROR = 1,
        WARNING = 2,
        INFO = 3
    };

    LogType type;
    std::string message;
};

class Console {
public:
    static void log(LogInfo::LogType type, const std::string &msg);
};

