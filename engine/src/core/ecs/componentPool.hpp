#pragma once

class Pool {
public:
    virtual void *GenerateNewInstance() = 0;
};

template <typename T>
class ComponentPool : public Pool {
public:
    inline void *GenerateNewInstance() override {
        return (void *)new T();
    }
};


