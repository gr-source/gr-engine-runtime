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

    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);
    data += sizeof(MemoryBlock);
    data = reinterpret_cast<uint8_t*>(ALIGN16(reinterpret_cast<std::uintptr_t>(data)));

    MemoryBlock* mem = reinterpret_cast<MemoryBlock*>(data - sizeof(MemoryBlock));
    mem->origPtr = ptr;
    mem->sz = size;

    return data;
}

void* Memory::realloc(void *ptr, std::size_t size) {
    if (!ptr) {
        return Memory::malloc(size);
    }

    MemoryBlock* mem = reinterpret_cast<MemoryBlock*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(MemoryBlock));
    uint32_t offset = reinterpret_cast<uint8_t*>(ptr) - reinterpret_cast<uint8_t*>(mem->origPtr);

    void* origPtr = std::realloc(mem->origPtr, size + sizeof(MemoryBlock) + 15);
    if (!origPtr) {
        throw std::bad_alloc();
    }
    uint8_t* newPtr = reinterpret_cast<uint8_t*>(origPtr) + sizeof(MemoryBlock);
    newPtr = reinterpret_cast<uint8_t*>(ALIGN16(reinterpret_cast<std::uintptr_t>(newPtr)));

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

    MemoryBlock *mem = reinterpret_cast<MemoryBlock*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(MemoryBlock));
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
