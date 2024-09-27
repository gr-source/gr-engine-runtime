#include "streaming.hpp"

Streaming::Streaming(const std::string &path, std::ios::openmode mode) : std::fstream(path, mode) {
    if (!is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
}

Streaming::~Streaming() {
    close();
}
