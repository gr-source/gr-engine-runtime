#pragma once

#include <sstream>
#include <string>
#include <mutex>
#include <list>

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
    static void Print(LogInfo::LogType type, const std::string &msg);

    template <typename T, typename ...Args>
    static void log(LogInfo::LogType type, T params, Args &&... args);

    static void log(LogInfo::LogType type);

    static std::list<LogInfo> GetListLogs();

    static bool bChanged;
    
private:
    static std::list<LogInfo> s_logs;

    static std::mutex m_mutex;

    static int s_maxLog;

    static std::ostringstream oss;
};

template <typename T, typename... Args>
inline void Console::log(LogInfo::LogType type, T params, Args &&...args) {
    oss << params << " ";
    log(type, std::forward<Args>(args)...);
}
