#include "core/string.hpp"

bool StringUTF::Contains(const std::string &str1, const std::string &str2) {
    return str1.find(str2) != std::string::npos;
}

std::string StringUTF::RemoveNumber(const std::string &text) {
    std:: string result;
    for (auto & ch : text) {
        if (std::isdigit(ch)) {
            continue;
        }
        result += ch;
    }
    return result;
}



