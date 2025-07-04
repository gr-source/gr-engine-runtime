#pragma once

#include <string>

class Project {
protected:
    static Project *m_current;

private:
    std::string m_name;
    std::string m_path;

public:
    static void SetCurrent(Project *project);

    static Project *GetCurrent();
    
    Project(const std::string &name, const std::string &path);

    std::string getPath() const;

    std::string getName() const;
};

