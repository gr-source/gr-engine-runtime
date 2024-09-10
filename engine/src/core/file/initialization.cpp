#include "initialization.hpp"

Initialization::Initialization(const std::string &path, std::ios_base::openmode mode) : m_stream(path, mode) {
    if (mode == std::ios_base::in) {
        readFile();
    }
}

Initialization::~Initialization() {
    if (!m_stream.is_open()) {
        return;
    }
    m_stream.close();
}

bool Initialization::is_open() {
    return m_stream.is_open();
}

bool Initialization::hasKey(const std::string &key) {
    return m_map.find(key) != m_map.end();
}

bool Initialization::empty() {
    return m_map.empty();
}

void Initialization::save() {
    for (auto &&[key, value] : m_map) {
        m_stream << key << "=" << value;
    }
}

template <>
void Initialization::setValue(const std::string &key, const float &value) {
    m_map[key] = std::to_string(value);
}

template <>
void Initialization::setValue(const std::string &key, const bool &value) {
    m_map[key] = std::to_string(static_cast<int>(value));
}

template <>
void Initialization::setValue(const std::string &key, const int &value) {
    m_map[key] = std::to_string(value);
}

template <>
void Initialization::setValue(const std::string &key, const std::string &value) {
    m_map[key] = value;
}

template <>
int Initialization::getValue(const std::string &key) {
    return std::atoi(m_map[key].c_str());
}

template <>
float Initialization::getValue(const std::string &key) {
    return std::atof(m_map[key].c_str());
}

template <>
std::string Initialization::getValue(const std::string &key) {
    return m_map[key];
}

void Initialization::readFile() {
    std::string line;
    while (std::getline(m_stream, line)) {
        auto start = line.find('=');
        if (start == std::string::npos) {
            continue;
        }
        auto key = line.substr(0, start);
        auto value = line.substr(start + 1, line.size());

        if (key.empty() || value.empty()) {
            continue;
        }
        m_map.emplace(key, value);
    }
}
