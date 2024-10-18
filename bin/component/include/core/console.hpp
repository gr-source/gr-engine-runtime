#pragma once

#include <sstream>
#include <string>

#include "string.hpp"

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

    template <typename ...Args>
    static void Message(LogInfo::LogType type, const std::string &fmt, Args &&...args);
};

template <typename... Args>
inline void Console::Message(LogInfo::LogType type, const std::string &fmt, Args &&...args) {
    log(type, StringUTF::format(fmt, std::forward<Args>(args)...));
}

