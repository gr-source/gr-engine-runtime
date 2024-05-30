#include "log.h"

#include <iostream>

const std::string Log::red    = "\033[91m";
const std::string Log::none   = "\033[39m";
const std::string Log::green  = "\033[92m";
const std::string Log::orange = "\033[93m";

std::ostream& Log::error() {
    return std::cout << red;
}

std::ostream& Log::info() {
    return std::cout << green;
}

std::ostream& Log::warning() {
    return std::cout << orange;
}

std::ostream& Log::end(std::ostream& os) {
    return std::flush(os.put(os.widen('\n'))) << Log::none;
}


