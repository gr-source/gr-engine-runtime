#pragma once

#include "texture.h"

class TextureSample : public Texture {
public:
    TextureSample();
    
    void apply(void* pixels = nullptr) override;

    void setsamples(u32 sample);
protected:
    u32 m_samples;
};