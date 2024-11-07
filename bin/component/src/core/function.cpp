#include "core/function.hpp"

Function &Function::GetInstance() {
    static Function m_function;
    return m_function;
}

std::unordered_map<std::string, void **> Function::GetFunctionRefrence() {
    return GetInstance().m_functions;
}

void Function::insert(std::string id, void **function) {
    if (m_functions.find(id) != m_functions.end()) {
        return;
    }
    m_functions[id] = function;
}
