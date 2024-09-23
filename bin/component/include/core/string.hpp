#pragma once

#include <string>

class StringUTF {
public:
    StringUTF() = delete;
    ~StringUTF() = delete;

    static bool Contains(const std::string &str1, const std::string &str2);

    static std::string RemoveNumber(const std::string &text);
};

