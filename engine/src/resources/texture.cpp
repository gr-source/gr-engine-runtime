#include "texture.h"

Texture::Texture(const std::string &path, u32 textureType) : m_textureType(textureType), path(path), m_texture(-1), m_width(0), m_height(0), m_hasMipmap(false), m_format(eNONE), m_type(eNONE), m_internalformat(eNONE), m_wrapMode(eREPEAT) {}

Texture::Texture(const GLuint &textureID, const std::string &path) : m_texture(textureID), path(path), m_hasMipmap(false), m_format(eNONE), m_type(eNONE), m_internalformat(eNONE), m_textureType(eTEXTURE2D), m_wrapMode(eREPEAT) {}

Texture::Texture(u32 width, u32 height, u32 internalFormat, u32 format, u32 filter, u32 type, bool hasMipmap) : m_texture(-1), m_width(width), m_height(height), m_internalformat(internalFormat), m_format(format), m_filter(filter), m_type(type), m_hasMipmap(hasMipmap), m_textureType(eTEXTURE2D), m_wrapMode(eREPEAT) {}

Texture::~Texture() {
    if (isvalid())
        GL_CHECK(glDeleteTextures(1, &m_texture));
}

void Texture::bind(GLenum texture) {
    m_usage = texture;

    GL_CHECK(glActiveTexture(m_usage));
    GL_CHECK(glBindTexture(m_textureType, m_texture));
}

void Texture::unbind() {
    GL_CHECK(glActiveTexture(m_usage));
    GL_CHECK(glBindTexture(m_textureType, 0));
}

void Texture::apply(void *pixels) {
    if (!isvalid()) {
        GL_CHECK(glGenTextures(1, &m_texture));
    }

    GL_CHECK(glBindTexture(m_textureType, m_texture));

    if (m_textureType != GL_TEXTURE_2D_MULTISAMPLE) {
        GL_CHECK(glTexImage2D(m_textureType, 0, m_internalformat, m_width, m_height, 0, m_format, m_type, pixels));

        GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_WRAP_S, m_wrapMode));
        GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_WRAP_T, m_wrapMode));

        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        GL_CHECK(glTexParameterfv(m_textureType, GL_TEXTURE_BORDER_COLOR, borderColor));

        if (m_hasMipmap) {
            GL_CHECK(glGenerateMipmap(m_textureType));

            switch (m_filter) {
            case Texture::eBILINEAR: {
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                break;
            }
            case Texture::eTRILINEAR: {
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
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
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                break;
            case Texture::ePOINT:
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                GL_CHECK(glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                break;
            default:
                assert(false && "Filtered mod invalid");
                break;
            }
        }
    } else {
        GL_CHECK(glTexImage2DMultisample(m_textureType, 4, m_internalformat, m_width, m_height, GL_TRUE));
    }
    glBindTexture(m_textureType, 0);
}

void Texture::resize(u32 width, u32 height, u32 internalFormat, u32 format, u32 filter, u32 type, bool hasMipmap, u32 wrapMode) {
    assert(width >= 0 || height >= 0 && "valor de altura e largura não pose ser menor que zero");
    m_width = width;
    m_height = height;
    m_internalformat = internalFormat;
    m_format = format;
    m_filter = filter;
    m_hasMipmap = hasMipmap;
    m_type = type;
    m_wrapMode = wrapMode;
}

void Texture::resize(u32 width, u32 height, u32 format, u32 filter, const bool hasMipmap, u32 type) {
    assert(width >= 0 || height >= 0 && "valor de altura e largura não pose ser menor que zero");
    m_width = width;
    m_height = height;
    m_format = format;
    m_filter = filter;
    m_hasMipmap = hasMipmap;
    m_type = type;
}

const std::string& Texture::getpath() const {
    return path;
}

bool Texture::isvalid() const {
    return m_texture != -1;
}

const u32 &Texture::getwidth() const {
    return m_width;
}

const u32 &Texture::getheight() const {
    return m_height;
}

const u32 &Texture::getinternalformat() const {
    return m_internalformat;
}

GLuint Texture::getID() {
    return m_texture;
}
