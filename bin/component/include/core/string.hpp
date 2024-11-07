#pragma once

#include <string>

class StringUTF {
public:
    StringUTF() = delete;
    ~StringUTF() = delete;

    static std::string replace_all(std::string text, const std::string &lhs, const std::string &rhs);

    static bool Contains(const std::string &str1, const std::string &str2);

    static std::string RemoveNumber(const std::string &text);

    template <typename ...Args>
    static std::string format(const std::string &fmt, Args &&...args);
};

template <typename... Args>
inline std::string StringUTF::format(const std::string &fmt, Args &&...args) {
    auto retVal = snprintf(nullptr, 0, fmt.c_str(), std::forward<Args>(args)...);
    if (retVal <= 0) {
        return "";
    }
    std::string buffer(retVal, '\0');
    snprintf(buffer.data(), buffer.size() + 1, fmt.c_str(), std::forward<Args>(args)...);
    return buffer;
}




