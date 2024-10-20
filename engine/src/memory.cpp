#include "memory.h"

#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <cstring>
#include <cassert>

void Memory::Initialize() {}

void Memory::Shutdown() {}

#define JMP_OFFSET 5

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

int hook_replace_inst(void *orig_func, void *hook_func) {
    uint8_t jumpInstruction[14] = {0};
    jumpInstruction[0] = 0x49; // REX prefix
    jumpInstruction[1] = 0xBB; // MOV r11, imm64
    *reinterpret_cast<int64_t *>(jumpInstruction + 2) = reinterpret_cast<int64_t>(hook_func); // Set the hook function address
    jumpInstruction[10] = 0x41; // REX prefix
    jumpInstruction[11] = 0xFF; // JMP r11
    jumpInstruction[12] = 0xE3; // This is incorrect for jumping back, so we will change the jump instruction
    
    std::memcpy(orig_func, jumpInstruction, sizeof(jumpInstruction));
    return 0;
}

void *get_page_addr(void *addr) {
    return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(addr) & ~(getpagesize() - 1));
}

std::size_t calculate_function_size(void *func_start) {
    uint8_t *current = static_cast<uint8_t *>(func_start);
    std::size_t size = 0;

    while (size < 500) {
        if (current[size] == 0xC3) {
            break;
        }
        size++;
    }

    return size;
}

int Memory::hook(void *orig_func, void *new_func, void **out_orig) {
    auto func_size = calculate_function_size(orig_func);

    auto orig_bytes = new uint8_t[func_size];
    std::memcpy(orig_bytes, orig_func, func_size);

    if (out_orig) {
        *out_orig = orig_bytes;
    }

    void *page_addr = get_page_addr(orig_func);
    if (mprotect(page_addr, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        delete[] orig_bytes;
        return 1;
    }

    if (hook_replace_inst(orig_func, new_func) != 0) {
        mprotect(page_addr, getpagesize(), PROT_READ | PROT_EXEC);
        delete[] orig_bytes;
        return 2;
    }

    if (mprotect(page_addr, getpagesize(), PROT_READ | PROT_EXEC) == -1) {
        delete[] orig_bytes;
        return 3;
    }
    delete[] orig_bytes;

    return 0;
}

