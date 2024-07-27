#include "console.hpp"

std::list<LogInfo> Console::s_logs;

int Console::s_maxLog = 20;

void Console::Print(LogInfo::LogType type, const std::string &msg) {
    s_logs.push_back({type, msg});

    if (s_logs.size() >= s_maxLog) {
        s_logs.pop_front();
    }
}

void Console::Print(const std::string &msg) {
    s_logs.push_back({LogInfo::LogType::INFO, msg});

    if (s_logs.size() >= s_maxLog) {
        s_logs.pop_front();
    }
}

std::list<LogInfo> &Console::GetRegisterLog() {
    return s_logs;
}
