#include "stream.hpp"

Streaming::Streaming(const std::string &path, std::ios_base::openmode mode) : m_path(path), std::fstream(path, mode) {}

Streaming::Streaming() : std::fstream() { }

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
        std::fstream::write(reinterpret_cast<const char *>(data), size);

        std::free(data);
    }
    m_data.clear();
}

bool Streaming::write(const void *data, std::size_t size) {
    if (!data || size <= 0) {
        return false;
    }
    auto ptr = std::malloc(size);

    if (!std::memcpy(ptr, data, size)) {
        return false;
    }

    m_data.emplace_back(ptr, static_cast<std::streamsize>(size));

    return true;
}

bool Streaming::read(void *data, std::size_t size) {
    if (std::fstream::read(reinterpret_cast<char *>(data), static_cast<std::streamsize>(size))) {
        return true;
    }
    return false;
}

bool Streaming::is_open() const {
    return std::fstream::is_open();
}

std::streampos Streaming::tellg() {
    return std::fstream::tellg();
}

std::istream &Streaming::seekg(std::streamoff position, std::ios_base::seekdir mode) {
    return std::fstream::seekg(position, mode);
}

void Streaming::clear() {
    for (auto &&[data, size] : m_data) {
        std::free(data);
    }
    m_data.clear();
}

std::string Streaming::getPath() const {
    return m_path;
}
