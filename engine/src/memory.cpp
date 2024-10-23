#include "memory.h"

#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <cstring>
#include <cassert>

#pragma pack(push, 1)

struct subhook_jmp32 {
  uint8_t opcode;
  int32_t offset;
};

/* Since AMD64 doesn't support 64-bit direct jumps, we'll push the address
 * onto the stack, then call RET.
 */
struct subhook_jmp64 {
  uint8_t  push_opcode;
  uint32_t push_addr; /* lower 32-bits of the address to jump to */
  uint8_t  mov_opcode;
  uint8_t  mov_modrm;
  uint8_t  mov_sib;
  uint8_t  mov_offset;
  uint32_t mov_addr;  /* upper 32-bits of the address to jump to */
  uint8_t  ret_opcode;
};

#pragma pack(pop)

void Memory::Initialize() {}

void Memory::Shutdown() {}

int hook_replace_inst(void *orig_func, void *hook_func) {
    struct subhook_jmp64 *jmp = (struct subhook_jmp64 *)orig_func;

    jmp->push_opcode = 0x68;
    jmp->push_addr = (uint32_t)(uintptr_t)hook_func;
    jmp->mov_opcode = 0xC7;
    jmp->mov_modrm = 0x44;
    jmp->mov_sib = 0x24;
    jmp->mov_offset = 0x04;
    jmp->mov_addr = (uint32_t)(((uintptr_t)hook_func) >> 32);
    jmp->ret_opcode = 0xC3;
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
    void *page_addr = get_page_addr(orig_func);
    if (mprotect(page_addr, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        return 1;
    }

    if (hook_replace_inst(orig_func, new_func) != 0) {
        mprotect(page_addr, getpagesize(), PROT_READ | PROT_EXEC);
        return 2;
    }

    if (mprotect(page_addr, getpagesize(), PROT_READ | PROT_EXEC) == -1) {
        return 3;
    }

    return 0;
}

