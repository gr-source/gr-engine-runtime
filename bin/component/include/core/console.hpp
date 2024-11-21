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
    static void (*log)(LogInfo::LogType type, const std::string &msg);

    template <typename ...Args>
    static void Message(LogInfo::LogType type, const std::string &fmt, Args &&...args);

    template <typename ...Args>
    static void LogWarning(const std::string &format, Args &&...args);

    template <typename ...Args>
    static void LogError(const std::string &format, Args &&...args);

    template <typename ...Args>
    static void LogInfo(const std::string &format, Args &&...args);
};

template <typename... Args>
inline void Console::Message(LogInfo::LogType type, const std::string &fmt, Args &&...args) {
    if constexpr (sizeof...(Args) > 0) {
        return log(type, StringUTF::format(fmt, std::forward<Args>(args)...));
    }
    return log(type, fmt);
}

template <typename... Args>
inline void Console::LogWarning(const std::string &format, Args &&...args) {
    return Console::Message(LogInfo::WARNING, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::LogError(const std::string &format, Args &&...args) {
    return Console::Message(LogInfo::ERROR, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::LogInfo(const std::string &format, Args &&...args) {
    return Console::Message(LogInfo::INFO, format, std::forward<Args>(args)...);
}





