#pragma once

#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <tuple>

class Streaming : private std::fstream {
public:
    Streaming(const std::string &path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    ~Streaming() override;

    template <typename T>
    bool writeS(const T *data, std::streamsize count, bool bCache = true);

    template <typename T>
    bool readS(T *data, std::streamsize count);

    std::streamsize getDataSize();

    void flush();

    bool write(const void *data, std::size_t size);

	bool read(void *data, std::size_t size);

    bool is_open() const;

    std::streampos tellg();

    std::istream &seekg(std::streamoff position, std::ios_base::seekdir mode);
    
private:
    std::vector<std::tuple<void *, std::streamsize>> m_data;
};

template <typename T>
bool Streaming::writeS(const T *data, std::streamsize count, bool bCache) {
    auto size = count * sizeof(T);
    if (!bCache) {
        if (std::fstream::write(reinterpret_cast<const char *>(data), size)) {
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
bool Streaming::readS(T *data, std::streamsize count) {
    return read(reinterpret_cast<void *>(data), static_cast<std::size_t>(count * sizeof(T)));
}




