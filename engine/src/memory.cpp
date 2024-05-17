#include "memory.h"

size memory::totalMemoryUsage = 0;

void *memory::malloc(const size &sz) {
    void* data = std::malloc(sz + sizeof(MemoryBlock));
    if (!data) {
        throw std::bad_alloc();
    }

    totalMemoryUsage += sz;

    MemoryBlock* block = new (data) MemoryBlock();
    block->origPtr = data;
    block->sz = sz;

    // Retornar um ponteiro para a área de dados após o bloco de controle
    return static_cast<void*>(static_cast<char*>(data) + sizeof(MemoryBlock));
}

void memory::free(void *ptr) noexcept {
    if (!ptr) return;

    MemoryBlock *mem = (MemoryBlock*)((std::uint8_t*)ptr - sizeof(MemoryBlock));
    totalMemoryUsage -= mem->sz;

    std::free(mem->origPtr);
}

void *operator new(size sz) {
    return memory::malloc(sz);
}

void operator delete(void *ptr) noexcept {
    memory::free(ptr);
}
