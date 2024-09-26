#pragma once

#include <unordered_map>
#include <string>

class Config {
public:
    Config(const std::string &path = "");

    void load();

    void save();

    bool hasKey(const std::string &key);

    template <typename T>
    T setValue(const std::string &key, T value);
    
    template <typename T>
    T getValue(const std::string &key);
    
    const std::unordered_map<std::string, std::string> &getData() const;

    bool operator [](const std::string &key) const;

private:
    std::unordered_map<std::string, std::string> m_keyMap;

    std::string m_path;
};

