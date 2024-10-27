#pragma once

#include <sstream>
#include <string>
#include <mutex>
#include <list>

#include "core/string.hpp"

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

    static std::list<LogInfo> GetListLogs();

    static bool bChanged;

    static void Shutdown();

    template <typename ...Args>
    static void Message(LogInfo::LogType type, const std::string &format, Args &&...args);
    
private:
    static std::list<LogInfo> m_logs;

    static std::mutex m_mutex;

    static int m_maxLog;
};

#define LOG_WARNING_ARGS(fmt, args...) Console::Message(LogInfo::WARNING, fmt, args)

#define LOG_WARNING(msg) Console::log(LogInfo::WARNING, msg)

#define LOG_ERROR_ARGS(fmt, args...) Console::Message(LogInfo::ERROR, fmt, args)

#define LOG_ERROR(msg) Console::log(LogInfo::ERROR, msg)

#define LOG_INFO_ARGS(fmt, args...) Console::Message(LogInfo::INFO, fmt, args)

#define LOG_INFO(msg) Console::log(LogInfo::INFO, msg)


template <typename... Args>
inline void Console::Message(LogInfo::LogType type, const std::string &format, Args &&...args) {
    return log(type, String::format(format, std::forward<Args>(args)...));
}
