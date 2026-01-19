#pragma once

#include <string>

class Project
{
public:
    std::string name;
    std::string path;
    std::string version;
    
    Project() = default;
    ~Project() = default;

    void save();

    bool load(const std::string& file);
};

