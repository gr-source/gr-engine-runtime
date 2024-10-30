#include "core/stream.hpp"
#include "core/stream.hpp"

#include <cstring>

Stream::Stream(const std::string &path) : m_path(path), std::fstream(path) {}

Stream::Stream(const std::string &path, std::ios_base::openmode mode) : m_path(path), std::fstream(path, mode) {}

bool Stream::write(const void *data, std::size_t size) {
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

bool Stream::read(void *data, std::size_t size) {
    if (std::fstream::read(reinterpret_cast<char *>(data), static_cast<std::streamsize>(size))) {
        return true;
    }
    return false;
}



