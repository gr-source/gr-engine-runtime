#include "timer.h"
#include "Global.h"

float Time::deltaTime = 0.0f;

int64_t difftimespec_ns(const struct timespec after, const struct timespec before) {
    return ((int64_t)after.tv_sec - (int64_t)before.tv_sec) * (int64_t)1000000000 + ((int64_t)after.tv_nsec - (int64_t)before.tv_nsec);
    // clock_gettime(CLOCK_MONOTONIC, &endTime);
}

Time::Time(float fps) : dt(0.0f), targetFPS(1.0f / fps), lastTime(std::chrono::steady_clock::now()) {}

float Time::start() const {
    auto startTime = std::chrono::steady_clock::now();

    auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(startTime - endTime);

    return static_cast<float>(elapsedTime.count() / 1e6f);
}

void Time::end() {
    endTime = std::chrono::steady_clock::now();
}

void Time::tick() {
    auto currentTime = std::chrono::steady_clock::now();

    auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);

    auto sleepTime = targetFPS - elapsedTime;

    if (sleepTime >= std::chrono::microseconds(0)) {
        dt = std::abs(sleepTime.count() / 1e6f);

        std::this_thread::sleep_for(sleepTime);
    }

    lastTime = std::chrono::steady_clock::now();
}
