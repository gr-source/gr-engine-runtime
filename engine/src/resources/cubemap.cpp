#include "cubemap.h"

/*
Cubemap::Cubemap(u32 width, u32 height, u32 internalFormat, u32 format, u32 filter, u32 type, bool m_hasMipmap) : Texture(width, height, internalFormat, format, filter, type, m_hasMipmap) {
    GL_CHECK(glGenTextures(1, &m_texture));

    m_textureType = GL_TEXTURE_CUBE_MAP;
}

void Cubemap::apply(u32 target, void* pixels) {
    if (m_texture == -1) GL_CHECK(glGenTextures(1, &m_texture));

    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture));

    // u32 itf = (m_internalformat != Texture::eNONE ? m_internalformat : getsrgbformat());
    // u32 f = (m_format != Texture::eNONE ? m_format : getinternalformat());
    // u32 t = (m_type != Texture::eNONE ? m_type : Texture::eBYTE);

    GL_CHECK(glTexImage2D(target, 0, m_internalformat, m_width, m_height, 0, m_format, m_type, pixels));

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (m_hasMipmap) {
        switch (m_filter) {
        case Texture::eBILINEAR: {
            GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            break;
        }
        case Texture::eTRILINEAR: {
            GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
            GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            break;
        }
        default:
            assert(false && "Filtered mod invalid");
            break;
        }
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
    } else {
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
*/




