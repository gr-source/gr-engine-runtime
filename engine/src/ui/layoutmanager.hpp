#pragma once

#include "ui/viewlayout.hpp"

#include <memory>
#include <tuple>
#include <utility>
#include <vector>

class LayoutManager
{
public:
    template <typename T, typename ...Args>
    static ViewLayout *add(const char *name, Args &&...args);

private:
    static std::vector<std::tuple<const char *, std::unique_ptr<ViewLayout>>> layouts;
};

template <typename T, typename ...Args>
inline ViewLayout *LayoutManager::add(const char *name, Args &&...args)
{
    auto &&[n, view] = layouts.emplace_back(name, std::make_unique<T>(std::forward<Args>(args)...));
    return view;
}


