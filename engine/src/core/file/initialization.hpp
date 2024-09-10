#pragma once

#include <unordered_map>
#include <fstream>
#include <string>

class Initialization {
public:
    Initialization(const std::string &path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    ~Initialization();

    bool is_open();

    template <typename T>
    void setValue(const std::string &key, const T &value);

    template <typename T>
    T getValue(const std::string &key);

    bool hasKey(const std::string &key);

    bool empty();

    void save();

private:
    std::unordered_map<std::string, std::string> m_map;

    std::fstream m_stream;

    void readFile();
};


