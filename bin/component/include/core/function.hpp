#pragma once

#include <unordered_map>
#include <string>

class Function {
public:
    static Function &GetInstance();

    static std::unordered_map<std::string, void **> GetFunctionRefrence();

    void insert(std::string id, void **function);

private:
    std::unordered_map<std::string, void **> m_functions;
};

#define INSERT_FUNCTION(id, func) \
    static bool register_function_##id = (Function::GetInstance().insert(#func, (void **)&func), true);






