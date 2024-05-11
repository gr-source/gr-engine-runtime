#include "texture2D.h"

Texture2D::Texture2D() : Texture(-1) {
    m_textureType = GL_TEXTURE_2D;
}

Texture2D::Texture2D(const GLuint& textureID, const std::string& path) : Texture(textureID, path) {
    m_textureType = GL_TEXTURE_2D;
}

void Texture2D::apply(void* pixels) {
    if (!isvalid()) {
        GL_CHECK(glGenTextures(1, &m_texture));
    }

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_texture));

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, m_internalformat, m_width, m_height, 0, m_format, m_type, pixels));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapMode));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapMode));

    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    GL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));

    if (m_hasMipmap) {
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

        switch (m_filter) {
        case Texture::eBILINEAR: {
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            break;
        }
        case Texture::eTRILINEAR: {
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            break;
        }
        default:
            assert(false && "Filtered mod invalid");
            break;
        }
    } else {
        switch (m_filter) {
        case Texture::eTRILINEAR:
        case Texture::eBILINEAR:
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            break;
        case Texture::ePOINT:
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            break;
        default:
            assert(false && "Filtered mod invalid");
            break;
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

