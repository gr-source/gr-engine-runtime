#include "core/string.hpp"

std::string StringUTF::replace_all(std::string text, const std::string &lhs, const std::string &rhs){
    std::size_t pos = 0;
    while (true) {
        pos = text.find(lhs, pos);
        if (pos == std::string::npos) {
            break;
        }

        text = text.replace(pos, lhs.size(), rhs);
        pos += rhs.size();
    }

    return text;
}

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



