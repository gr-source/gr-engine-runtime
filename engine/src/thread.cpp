#include "thread.h"

#include <iostream>

Thread::~Thread() {
    if (m_thread.joinable()) {
        m_thread.join();

        std::cout << "[ Thread ] " << m_name << " Termined" << std::endl;
    }
}

void Thread::join() {
    if (m_thread.joinable()) {
        m_thread.join();

        std::cout << "[ Thread ] " << m_name << " Termined" << std::endl;
    }
}
