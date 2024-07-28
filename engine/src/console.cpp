#include "console.hpp"

std::list<LogInfo> Console::s_logs;

int Console::s_maxLog = 500;

bool Console::bChanged = false;

void Console::Print(LogInfo::LogType type, const std::string &msg) {
    s_logs.push_back({type, msg});

    if (s_logs.size() >= s_maxLog) {
        s_logs.pop_front();
    }
    Console::bChanged = true;
}

std::list<LogInfo> &Console::GetListLogs() {
    return s_logs;
}
