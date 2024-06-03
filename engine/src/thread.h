#pragma once

#include <string>
#include <thread>

class Thread {
public:
    template <typename Callback, typename... Args>
    explicit Thread(const std::string& name, Callback&& callback, Args&&... args);

    ~Thread();

    void join();
private:
    std::string m_name;

    std::thread m_thread;
};

template <typename Callback, typename... Args>
inline Thread::Thread(const std::string &name, Callback &&callback, Args &&...args) : m_name(name) {
    m_thread = std::thread(std::forward<Callback>(callback), std::forward<Args>(args)...);
}
