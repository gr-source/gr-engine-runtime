#include "sprite.h"

Sprite::Sprite(const std::string& path) : Texture(path) {
    m_textureType = GL_TEXTURE_2D;
}

Sprite::Sprite(const GLuint& textureID, const std::string& path) : Texture(textureID, path) {
    m_textureType = GL_TEXTURE_2D;
}

void Sprite::apply(void *pixels) {
    if (m_texture == -1) GL_CHECK(glGenTextures(1, &m_texture));

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_texture));

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, m_internalformat, m_width, m_height, 0, m_format, m_type, pixels));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    if (m_hasMipmap) {
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    } else {
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}
