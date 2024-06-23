#pragma once

#include <thread>

#include "eventManager.h"

template <typename Result, typename... Params>
class TaskRuntime {
public:
    void run(Params&&... params);

private:
    virtual Result doInBackground(Params&&... params) = 0;

    virtual void onPostExecute(Result& result) {}

protected:
    std::thread m_thread;

    void handle(Params&&... params) {
        // std::unique_lock<std::mutex> lock(m_mutex);
        Result ptr = doInBackground(std::forward<Params>(params)...);
        // lock.unlock();

        EventManager::addEvent(&TaskRuntime::finish, this, ptr);
    }

    void finish(Result& result) {
        if (m_thread.joinable()) {
            m_thread.join();
        }

        onPostExecute(result);
    }

    ~TaskRuntime();

    std::mutex m_mutex;
};

// template <typename Result, typename... Params>
// inline void TaskRuntime<Params, ...Result>::run(Params params[]) {
//     m_thread = std::thread(&TaskRuntime::handle, this, params);
// }

template <typename Result, typename... Params>
inline void TaskRuntime<Result, Params...>::run(Params &&...params) {
    m_thread = std::thread(&TaskRuntime::handle, this, std::forward<Params>(params)...);
}

template <typename Result, typename... Params>
inline TaskRuntime<Result, Params...>::~TaskRuntime() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
