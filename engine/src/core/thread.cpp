#include "thread.hpp"

bool Thread::bRequestDetach = false;
std::mutex Thread::s_mutex;

void Thread::Stop() {
    std::lock_guard<std::mutex> lock(s_mutex);
    bRequestDetach = true;
}

const bool Thread::IsDetach() {
    std::lock_guard<std::mutex> lock(s_mutex);
    return bRequestDetach;
}

Thread::Thread() {}

Thread::~Thread() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Thread::stop() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
