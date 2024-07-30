#pragma once

#include <functional>
#include <thread>
#include <mutex>

class Thread {
public:
    static void Stop();

    static const bool IsDetach();

    Thread();
    ~Thread();

    template <typename ...Args>
    void run(Args &&...args);

    void stop();
private:
    std::thread m_thread;

    static bool bRequestDetach;

    static std::mutex s_mutex;
};

template <typename... Args>
inline void Thread::run(Args &&...args) {
    m_thread = std::thread(std::forward<Args>(args)...);
}
