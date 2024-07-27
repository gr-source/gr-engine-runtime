#pragma once

#include <string>
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

    static void Print(const std::string &msg);

    static std::list<LogInfo> &GetRegisterLog();
    
private:
    static std::list<LogInfo> s_logs;

    static int s_maxLog;
};

