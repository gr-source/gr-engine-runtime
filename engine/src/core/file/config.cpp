#include "config.hpp"

#include <sstream>
#include <fstream>

#include <iostream>

Config::Config(const std::string &path) : m_path(path) {}

void Config::load() {
    std::ifstream is(m_path);
    if (!is.is_open()) {
        throw std::runtime_error("Falied to open config: " + m_path);
    }

    std::string line;
    while (std::getline(is, line)) {
        std::stringstream iss(line);

        char c;
        if (!(iss >> c) || c == ';') {
            continue;
        }
        auto start = line.find('=');

        auto key = line.substr(0, start);
        auto value = line.substr(start + 1);

        m_keyMap.emplace(key, value);
    }
    is.close();
}

void Config::save() {
    std::ofstream os(m_path);
    for (auto &&[key, value] : m_keyMap) {
        os << key << "=" << value << std::endl;
    }
    os.close();
}

bool Config::hasKey(const std::string &key) {
    return m_keyMap.find(key) != m_keyMap.end();
}

const std::unordered_map<std::string, std::string> &Config::getData() const {
    return m_keyMap;
}

bool Config::operator[](const std::string &key) const {
    return m_keyMap.find(key) != m_keyMap.end();
}

// SET
template <>
std::string Config::setValue(const std::string &key, std::string value) {
    m_keyMap[key] = value;
    return value;
}

template <>
int Config::setValue(const std::string &key, int value) {
    m_keyMap[key] = std::to_string(value);
    return value;
}

template <>
float Config::setValue(const std::string &key, float value) {
    m_keyMap[key] = std::to_string(value);
    return value;
}

template <>
bool Config::setValue(const std::string &key, bool value) {
    m_keyMap[key] = std::to_string(static_cast<int>(value));
    return value;
}

// GET
template <>
std::string Config::getValue(const std::string &key) {
    return m_keyMap[key];
}

template <>
int Config::getValue(const std::string &key) {
    return std::atoi(m_keyMap[key].c_str());
}

template <>
float Config::getValue(const std::string &key) {
    return std::atof(m_keyMap[key].c_str());
}

template <>
bool Config::getValue(const std::string &key) {
    return static_cast<bool>(std::atoi(m_keyMap[key].c_str()));
}


