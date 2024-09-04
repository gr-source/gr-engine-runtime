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
    static void log(LogInfo::LogType type, const std::string &msg);

    static std::list<LogInfo> GetListLogs();

    static bool bChanged;

private:
    static std::list<LogInfo> s_logs;

    static std::mutex m_mutex;

    static int s_maxLog;
};

