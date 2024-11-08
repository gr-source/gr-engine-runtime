#include "core/console.hpp"

#include "core/function.hpp"

void (*Console::log)(LogInfo::LogType type, const std::string &msg);
INSERT_FUNCTION(0, Console::log)


