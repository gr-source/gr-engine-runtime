#pragma once

#include <mutex>

class Memory {
public:
    static std::size_t totalMemoryUsage;

    static std::mutex memoryMutex;

    static void Initialize();

    static void Shutdown();

    static void* malloc(std::size_t size);

    static void* realloc(void *ptr, std::size_t size);

    static void free(void* ptr) noexcept;

    static void hook(void *orig_func, void *new_func, void **out_orig);

private:
    
    struct MemoryBlock {
        std::size_t sz;
        void *origPtr;
    };

    static int s_handle;
};

