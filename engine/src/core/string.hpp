#pragma once

#include <string>

class String {
public:
    static std::string RemoveNumber(const std::string &text);

    static std::string replace_all(std::string text, const std::string &lhs, const std::string &rhs);

    static bool contains(const std::string &text, const std::string &value);
};



