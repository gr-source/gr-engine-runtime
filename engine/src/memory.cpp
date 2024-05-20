#include "memory.h"

#define ALIGN16(x) ((x) + 0xF & ~0xF)

std::size_t Memory::totalMemoryUsage = 0;

void *Memory::malloc(std::size_t size) {
    void* ptr = std::malloc(size + sizeof(MemoryBlock) + 15);
    if (!ptr) {
        throw std::bad_alloc();
    }

    totalMemoryUsage += size;

    u8* data = (u8*)ptr;
	data += sizeof(MemoryBlock);
	data = (u8*)ALIGN16((std::uintptr_t)data);

	MemoryBlock* mem = (MemoryBlock*)(data-sizeof(MemoryBlock));
    mem->origPtr = ptr;
    mem->sz = size;

    return data;
}

void* Memory::realloc(void *ptr, std::size_t size) {
	if (!ptr)
		return Memory::malloc(size);

	MemoryBlock* mem = (MemoryBlock*)((u8*)ptr - sizeof(MemoryBlock));
	u32 offset = (u8*)ptr - (u8*)mem->origPtr;

	void* origPtr = std::realloc(mem->origPtr, size + sizeof(MemoryBlock) + 15);
	if (!origPtr) {
		throw std::bad_alloc();
	}
	ptr = (u8*)origPtr + offset;
	mem = (MemoryBlock*)((u8*)ptr - sizeof(MemoryBlock));

	totalMemoryUsage -= mem->sz;
	mem->sz = size;
	mem->origPtr = origPtr;
	totalMemoryUsage += mem->sz;

	return ptr;
}

void Memory::free(void *ptr) noexcept {
    if (!ptr) return;

    MemoryBlock *mem = (MemoryBlock*)((std::uint8_t*)ptr - sizeof(MemoryBlock));
    totalMemoryUsage -= mem->sz;

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
