#pragma once

#include <fstream>
#include <string>

class Streaming : public std::fstream {
public:
    Streaming(const std::string &path, std::ios::openmode mode);
    ~Streaming();
};
