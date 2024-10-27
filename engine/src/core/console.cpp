#include "console.hpp"

std::list<LogInfo> Console::m_logs;

bool Console::bChanged = false;

int Console::m_maxLog = 500;

std::mutex Console::m_mutex;

void Console::log(LogInfo::LogType type, const std::string &msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.push_back({type, msg});

    if (m_logs.size() >= m_maxLog) {
        m_logs.pop_front();
    }
    Console::bChanged = true;
}

std::list<LogInfo> Console::GetListLogs() {
    std::list<LogInfo> logs;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        logs = m_logs;
    }
    return logs;
}

void Console::Shutdown() {
    bChanged = false;

    m_logs.clear();
}
