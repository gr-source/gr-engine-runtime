#pragma once

class Memory {
public:
    static void Initialize();

    static void Shutdown();

    static int hook(void *orig_func, void *new_func, void **out_orig);
};

