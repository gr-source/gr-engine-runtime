#include "textureSample.h"

TextureSample::TextureSample() : Texture(-1), m_samples(0) {
    m_textureType = GL_TEXTURE_2D_MULTISAMPLE;
}

void TextureSample::apply(void *pixels) {
    if (!isvalid()) GL_CHECK(glGenTextures(1, &m_texture));

    assert(m_samples >= 0 && "[ TextureSample::apply(void *pixels) ] Samples não pode ser menor que zero");

    GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texture));

    u32 internalFormat = (m_internalformat != Texture::eNONE ? m_internalformat : Texture::eSRBG);

    GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, internalFormat, m_width, m_height, GL_TRUE));
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void TextureSample::setsamples(u32 sample) {
    m_samples = sample;
}
