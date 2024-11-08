#pragma once

#include <functional>

class Threadpool {
public:
    Threadpool() = delete;
    ~Threadpool() = delete;

    static void (*CreateInternalTask)(std::function<void()> func);

    template <typename ...Args>
    static void AddTask(Args &&...args);
};

template <typename... Args>
inline void Threadpool::AddTask(Args &&...args) {
    return CreateInternalTask(std::bind(std::forward<Args>(args)...));
}




