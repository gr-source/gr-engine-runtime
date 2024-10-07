#include "config.hpp"

#include <filesystem>
#include <sstream>
#include <fstream>

#include <iostream>

Config::Config(const std::string &path) : m_path(path), m_category("none") {}

bool Config::is_open() const {
    return std::filesystem::exists(m_path);
}

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

        if (readSection(line)) {
            continue;
        }

        readProperty(line);
    }
    is.close();
}

void Config::save() {
    std::ofstream os(m_path);

    for (auto &&[name, property] : m_sections) {
        os << "[" << name << "]" << std::endl;

        for (auto &&[key, value] : property) {
            os << key << "=" << value << std::endl;
        }
    }
    os.close();
}

void Config::clear() {
    m_sections.clear();
}

void Config::setCategory(const std::string &name) {
    m_sections.emplace(name, std::unordered_map<std::string, std::string>());

    m_category = name;
}

std::string Config::getCategory() {
    return m_category;
}

bool Config::hasKey(const std::string &key) {
    return m_sections[m_category].find(key) != m_sections[m_category].end();
}

void Config::print() {
    for (auto &&[name, property] : m_sections) {
        std::cout << "[" << name << "]" << std::endl;

        for (auto &&[ key, value] : property) {
            std::cout << key << "=" << value << std::endl;
        }
    }
}

bool Config::readSection(const std::string &line) {
    auto start = line.find('[');
    if (start == std::string::npos) {
        return false;
    }

    auto end = line.find(']');
    if (end == std::string::npos) {
        return false;
    }

    auto name = line.substr(start + 1, end - 1);
    if (name.empty()) {
        return false;
    }
    setCategory(name);
    return true;
}

bool Config::readProperty(const std::string &line) {
    auto start = line.find('=');
    if (start == std::string::npos) {
        return false;
    }

    auto key = line.substr(0, start);
    auto value = line.substr(start + 1);

    m_sections[m_category].emplace(key, value);
    return true;
}

// SET
template <>
std::string Config::setValue(const std::string &key, std::string value) {
    m_sections[m_category][key] = value;
    return value;
}

template <>
int Config::setValue(const std::string &key, int value) {
    m_sections[m_category][key] = std::to_string(value);
    return value;
}

template <>
float Config::setValue(const std::string &key, float value) {
    m_sections[m_category][key] = std::to_string(value);
    return value;
}

template <>
bool Config::setValue(const std::string &key, bool value) {
    m_sections[m_category][key] = std::to_string(static_cast<int>(value));
    return value;
}

// GET
template <>
std::string Config::getValue(const std::string &key) {
    return m_sections[m_category][key];
}

template <>
int Config::getValue(const std::string &key) {
    return std::atoi(m_sections[m_category][key].c_str());
}

template <>
float Config::getValue(const std::string &key) {
    return std::atof(m_sections[m_category][key].c_str());
}

template <>
bool Config::getValue(const std::string &key) {
    return static_cast<bool>(std::atoi(m_sections[m_category][key].c_str()));
}


