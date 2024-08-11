#include "console.hpp"

std::list<LogInfo> Console::s_logs;

bool Console::bChanged = false;

int Console::s_maxLog = 500;

std::mutex Console::m_mutex;

std::ostringstream Console::oss;

void Console::Print(LogInfo::LogType type, const std::string &msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    s_logs.push_back({type, msg});

    if (s_logs.size() >= s_maxLog) {
        s_logs.pop_front();
    }
    Console::bChanged = true;
}

void Console::log(LogInfo::LogType type) {
    oss << std::endl;

    Console::Print(type, oss.str());
    oss.clear();
}

std::list<LogInfo> Console::GetListLogs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return s_logs;
}
