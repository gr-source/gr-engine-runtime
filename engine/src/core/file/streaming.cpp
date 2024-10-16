#include "streaming.hpp"

Streaming::~Streaming() {
    close();
}

std::streamsize Streaming::getDataSize() {
    std::streamsize buffer_size = 0;
    for (auto &&[data, size] : m_data) {
        buffer_size += size;
    }
    return buffer_size;
}

void Streaming::flush() {
    for (auto &&[data, size] : m_data) {
        write(reinterpret_cast<const char *>(data), size);

        std::free(data);
    }
    m_data.clear();
}
