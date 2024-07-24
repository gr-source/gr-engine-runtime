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
    static const std::string red;
    static const std::string none;
    static const std::string green;
    static const std::string orange;

    static std::ostream& error();

    static std::ostream& info();

    static std::ostream& warning();

    static std::ostream& end(std::ostream& os);
};




