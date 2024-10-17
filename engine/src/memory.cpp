#include "memory.h"


#include <sys/mman.h>

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cassert>
#include <fcntl.h>

#define ALIGN16(x) (((x) + 0xF) & ~0xF)

std::size_t Memory::totalMemoryUsage = 0;

std::mutex Memory::memoryMutex;

int Memory::s_handle = 0;

void Memory::Initialize() {
    s_handle = open64("/proc/self/mem", O_RDONLY);
    if (!s_handle) {
        std::cout << "Falied to Initialize memory." << std::endl;
        return;
    }
}

void Memory::Shutdown() {
    if (s_handle) {
        close(s_handle);
    }
}

void *Memory::malloc(std::size_t size) {
    /*void* ptr = std::malloc(size + sizeof(MemoryBlock) + 15);
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

    return data;*/
    return std::malloc(size);
}

void* Memory::realloc(void *ptr, std::size_t size) {
    /*if (!ptr) {
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
    mem->sz = size;*/

    return std::realloc(ptr, size);
}

void Memory::free(void *ptr) noexcept {
    if (!ptr) return;

    /*MemoryBlock *mem = reinterpret_cast<MemoryBlock*>(reinterpret_cast<u8*>(ptr) - sizeof(MemoryBlock));
    {
        std::lock_guard<std::mutex> lock(memoryMutex);
        totalMemoryUsage -= mem->sz;
    }

    std::free(mem->origPtr);*/
    std::free(ptr);
}


void Memory::hook(void *orig_func, void *new_func, void **out_orig) {
    // DobbyHook(orig_func, new_func, out_orig);
    // inline_hook(orig_func, new_func);
    auto pageSizeToProtect = sysconf(_SC_PAGESIZE);
    auto start = reinterpret_cast<long>(orig_func);
    auto pageStart = start & ~(pageSizeToProtect - 1);

    if (mprotect(reinterpret_cast<void *>(pageStart), pageSizeToProtect, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        throw std::runtime_error("Failed to change page permission in memory.");
    }

    uint8_t jumpInstruction[14] = {0};
    jumpInstruction[0] = 0x49; // REX prefix
    jumpInstruction[1] = 0xBB; // MOV r11, imm32
    *reinterpret_cast<int64_t *>(jumpInstruction + 2) = reinterpret_cast<int64_t>(new_func);
    jumpInstruction[10] = 0x41; // REX prefix
    jumpInstruction[11] = 0xFF; // JMP r11
    jumpInstruction[12] = 0xE3; // RIP-Relative Jump

    if (!memcpy(orig_func, jumpInstruction, sizeof(jumpInstruction))) {
        throw std::runtime_error("Falied to copy instructions.");
    }

    if (mprotect(reinterpret_cast<void *>(pageStart), pageSizeToProtect, PROT_READ | PROT_EXEC) == -1) {
        throw std::runtime_error("Failed to roll back page permissions in memory.");
    }
}

