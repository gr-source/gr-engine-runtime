#include "texture.h"

Texture::Texture(const std::string& path) : path(path), m_texture(-1), m_width(0), m_height(0), m_hasMipmap(false), m_format(eNONE), m_type(eNONE), m_internalformat(eNONE), m_textureType(eNONE), m_wrapMode(eREPEAT) {}

Texture::Texture(const GLuint &textureID, const std::string &path) : m_texture(textureID), path(path), m_hasMipmap(false), m_format(eNONE), m_type(eNONE), m_internalformat(eNONE), m_textureType(eNONE), m_wrapMode(eREPEAT) {}

Texture::Texture(u32 width, u32 height, u32 internalFormat, u32 format, u32 filter, u32 type, bool hasMipmap) : m_texture(-1), m_width(width), m_height(height), m_internalformat(internalFormat), m_format(format), m_filter(filter), m_type(type), m_hasMipmap(hasMipmap), m_textureType(eNONE), m_wrapMode(eREPEAT) {}

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
