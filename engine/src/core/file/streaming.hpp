#pragma once

#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <tuple>

class Streaming : public std::fstream {
public:
    Streaming(const std::string &path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) : std::fstream(path, mode) {}
    Streaming() : std::fstream() {}
    ~Streaming() override;

    template <typename T>
    bool writeS(const T *data, std::streamsize count, bool bCache = true);

    template <typename T>
    std::istream &readS(T *data, std::streamsize count);

    std::streamsize getDataSize();

    void flush();
    
private:
    std::vector<std::tuple<void *, std::streamsize>> m_data;
};

template <typename T>
bool Streaming::writeS(const T *data, std::streamsize count, bool bCache) {
    auto size = count * sizeof(T);
    if (!bCache) {
        if (write(reinterpret_cast<const char *>(data), size)) {
            return true;
        }
        return false;
    }
    auto ptr = std::malloc(size);
    if (!ptr) {
        return false;
    }

    if (!std::memcpy(ptr, reinterpret_cast<const void *>(data), size)) {
        std::free(ptr);
        return false;
    }

    m_data.emplace_back(ptr, size);
    return true;
}

template <typename T>
std::istream &Streaming::readS(T *data, std::streamsize count) {
    return read(reinterpret_cast<char *>(data), count * sizeof(T));
}




