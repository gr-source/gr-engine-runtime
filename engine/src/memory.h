#pragma once

#include "types.h"

class memory {
public:
    static size totalMemoryUsage;

    static void* malloc(const size& sz);

    static void free(void* ptr) noexcept;

private:
    
    struct MemoryBlock {
        size sz;
        u32 hint;
        void *origPtr;
        const char *codeline;
        //LLLink inAllocList;
    };
};

void *operator new(size sz);

void operator delete(void* ptr) noexcept;