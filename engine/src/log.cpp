#include "log.h"

#include <iostream>

const std::string Log::red    = "\033[91m";
const std::string Log::none   = "\033[39m";
const std::string Log::green  = "\033[92m";
const std::string Log::orange = "\033[93m";

void Log::Write(LogType type, const std::string& append) {
    switch (type) {
    case LogType::eINFO:
        std::cout << green;
        break;
    case LogType::eWARNING:
        std::cout << orange;
        break;
    case LogType::eERROR:
        std::cout << red;
        break;
    default:
        break;
    }
    std::cout << append;
    
    std::cout << none << std::endl;
}

std::ostream& Log::error = std::cout << red;

std::ostream& Log::info = std::cout << green;

std::ostream& Log::warning = std::cout << orange;

std::ostream& Log::end(std::ostream& os) {
    return std::flush(os.put(os.widen('\n')));
}