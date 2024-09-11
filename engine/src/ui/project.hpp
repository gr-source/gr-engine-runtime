#pragma once

#include <string>

class Project {
protected:
    static Project *m_current;

private:
    std::string m_name;
    std::string m_path;
    
public:
    static Project *GetCurrent();
    
    Project(const std::string &name, const std::string &path);

    const std::string &getPath() const;

    const std::string &getName() const;

    void open();
};

