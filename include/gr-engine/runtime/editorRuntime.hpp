#pragma once

#include "runtime.hpp"

class EditorRuntime : public Runtime
{
public:
    EditorRuntime();
    ~EditorRuntime() override;

    void initialize() override;
};

