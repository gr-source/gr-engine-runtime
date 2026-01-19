#pragma once

/*
#include <cstdint>
template <typename T>
constexpr const char* GetTypeName()
{
    return __PRETTY_FUNCTION__;
}

constexpr uint64_t GenerateHash(const char* str) {
    uint64_t hash = 14695981039346656037UL;
    while (*str != '\0')
    {
        hash ^= (uint64_t)(unsigned char)(*str);
        hash *= 1099511628211UL;
        str++;
    }
    return hash;
}

template <typename T>
inline constexpr uint64_t GetClassHash()
{
    return GenerateHash(GetTypeName<T>());
}
*/
class ViewLayout
{
public:
    virtual ~ViewLayout() {}

    virtual const char* getName() const { return "Unknow"; }

    virtual void onFocus() {}

    virtual void onBlur() {}

    virtual void onUpdate() {}

    virtual void onRender3D() {}

    virtual void onRenderUI() {}

    virtual void drop(void *data) {}
};









