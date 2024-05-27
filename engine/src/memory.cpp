#include "memory.h"

std::size_t Memory::totalMemoryUsage = 0;
std::mutex Memory::memoryMutex;

#define ALIGN16(x) (((x) + 0xF) & ~0xF)

void *Memory::malloc(std::size_t size) {
    void* ptr = std::malloc(size + sizeof(MemoryBlock) + 15);
    if (!ptr) {
        throw std::bad_alloc();
    }

    {
        std::lock_guard<std::mutex> lock(memoryMutex);
        totalMemoryUsage += size;
    }

    u8* data = reinterpret_cast<u8*>(ptr);
    data += sizeof(MemoryBlock);
    data = reinterpret_cast<u8*>(ALIGN16(reinterpret_cast<std::uintptr_t>(data)));

    MemoryBlock* mem = reinterpret_cast<MemoryBlock*>(data - sizeof(MemoryBlock));
    mem->origPtr = ptr;
    mem->sz = size;

    return data;
}

void* Memory::realloc(void *ptr, std::size_t size) {
    if (!ptr) {
        return Memory::malloc(size);
    }

    MemoryBlock* mem = reinterpret_cast<MemoryBlock*>(reinterpret_cast<u8*>(ptr) - sizeof(MemoryBlock));

    u32 offset = reinterpret_cast<u8*>(ptr) - reinterpret_cast<u8*>(mem->origPtr);


    void* origPtr = std::realloc(mem->origPtr, size + sizeof(MemoryBlock) + 15);
    if (!origPtr) {
        throw std::bad_alloc();
    }
    u8* newPtr = reinterpret_cast<u8*>(origPtr) + offset;

    mem = reinterpret_cast<MemoryBlock*>(newPtr - sizeof(MemoryBlock));
    {
        std::lock_guard<std::mutex> lock(memoryMutex);
        totalMemoryUsage -= mem->sz;
        totalMemoryUsage += size;
    }
    mem->origPtr = origPtr;
    mem->sz = size;

    return newPtr;
}

void Memory::free(void *ptr) noexcept {
    if (!ptr) return;

    MemoryBlock *mem = reinterpret_cast<MemoryBlock*>(reinterpret_cast<u8*>(ptr) - sizeof(MemoryBlock));
    {
        std::lock_guard<std::mutex> lock(memoryMutex);
        totalMemoryUsage -= mem->sz;
    }

    std::free(mem->origPtr);
}

void *operator new(std::size_t size) {
    return Memory::malloc(size);
}

void *operator new[](std::size_t size) {
    return Memory::malloc(size);
}

void operator delete(void *ptr) noexcept {
    Memory::free(ptr);
}

void operator delete[](void *ptr) noexcept {
    Memory::free(ptr);
}
