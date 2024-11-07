#include "core/console.hpp"

#include "core/function.hpp"

void (*Console::log)(LogInfo::LogType type, const std::string &msg) = nullptr;
INSERT_FUNCTION(7, Console::log)


