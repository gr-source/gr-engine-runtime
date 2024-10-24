#pragma once

#include <unordered_map>
#include <string>

class Config {
public:
    Config(const std::string &path = "");
    
    bool is_open() const;

    void load();

    void save();

    void clear();

    void setCategory(const std::string &name);

    std::string getCategory();

    bool hasKey(const std::string &key);

    template <typename T>
    T setValue(const std::string &key, T value);
    
    template <typename T>
    T getValue(const std::string &key);
    
    // bool operator [](const std::string &key) const;

    void print();

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_sections;

    // std::unordered_map<std::string, std::string> m_keyMap;

    std::string m_category;

    std::string m_path;

    bool readSection(const std::string &line);

    bool readProperty(const std::string &line);
};

