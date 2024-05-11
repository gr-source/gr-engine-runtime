#pragma once

#include <fstream>
#include <string>

enum struct LogType {
    eINFO    = 1,
    eWARNING = 2,
    eERROR   = 3
};

class Log {
public:
    static void Write(LogType type, const std::string& append);

    static const std::string red;
    static const std::string none;
    static const std::string green;
    static const std::string orange;
};

#define INFO(msg) Log::Write(LogType::eINFO, msg)

#define ERROR(msg) Log::Write(LogType::eERROR, msg)

#define WARNING(msg) Log::Write(LogType::eWARNING, msg)


