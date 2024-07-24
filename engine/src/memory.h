#pragma once

#include "types.h"

#include <mutex>

class Memory {
public:
    static std::size_t totalMemoryUsage;

    static void* malloc(std::size_t size);

    static void* realloc(void *ptr, std::size_t size);

    static void free(void* ptr) noexcept;

    static std::mutex memoryMutex;

private:
    
    struct MemoryBlock {
        std::size_t sz;
        void *origPtr;
    };
};

void *operator new(std::size_t size);

void *operator new[](std::size_t size);

void operator delete(void* ptr) noexcept;

void operator delete[](void* ptr) noexcept;

