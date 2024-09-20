#include "console.hpp"

std::list<LogInfo> Console::s_logs;

bool Console::bChanged = false;

int Console::s_maxLog = 500;

std::mutex Console::m_mutex;

void Console::log(LogInfo::LogType type, const std::string &msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    s_logs.push_back({type, msg});

    if (s_logs.size() >= s_maxLog) {
        s_logs.pop_front();
    }
    Console::bChanged = true;
}

std::list<LogInfo> Console::GetListLogs() {
    std::list<LogInfo> logs;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        logs = s_logs;
    }
    return logs;
}
