#pragma once

#include "../gl.h"

enum TexturePBR {
    ALBEDO = 0,
    NORMAL = 1,
    ROUGHNESS = 2,
    AO = 3,
    METALLIC = 4,
    Non = 5
};

struct TextureProperty {
    u32 m_format;
};

class Texture {
public:
    enum : u32 {
        /* texture format */
        eNONE        = 1 << 1,
        eRG          = GL_RG,
        eRED         = GL_RED,

        eRGB         = GL_RGB,
        eRGB16F      = GL_RGB16F,
        eRGB32F      = GL_RGB32F,

        eRGBA        = GL_RGBA,
        eRGBA16F     = GL_RGBA16F,
        eRGBA32F     = GL_RGBA32F,

        eSRBG        = GL_SRGB,
        eSRBG8       = GL_SRGB8,
        eSRBG_APHA   = GL_SRGB_ALPHA,
        eSRBG8_APHA8 = GL_SRGB8_ALPHA8,
        eDEPTH_COMPONENT = GL_DEPTH_COMPONENT,

        /* texture filter */
        eBILINEAR  = 2 << 1,
        eTRILINEAR = 2 << 2,
        ePOINT = 2 << 3,

        eREPEAT = GL_REPEAT,
        eCLAMPEDGE = GL_CLAMP_TO_EDGE,
        eCLAMPBORDER = GL_CLAMP_TO_BORDER,

        /* texture type */
        eBYTE      = GL_UNSIGNED_BYTE,
        eFLOAT     = GL_FLOAT,

        eTEXTURE2D = GL_TEXTURE_2D,
        eTEXTURE2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE
    };

    Texture(const Texture&& texture);
    
    Texture(const std::string& path = "deafult", u32 textureType = eTEXTURE2D);

    Texture(const GLuint& textureID, const std::string& path = "none");

    Texture(u32 width, u32 height, u32 internalFormat = eRGB, u32 format = eRGB, u32 filter = eBILINEAR, u32 type = eBYTE, bool hasMipmap = false);
    
    ~Texture();

    void bind(GLenum texture = GL_TEXTURE0);

    void unbind();

    void apply(void* pixels = nullptr);

    void resize(u32 width, u32 height, u32 internalFormat, u32 format, u32 filter, u32 type, bool hasMipmap = false, u32 wrapMode = eREPEAT);

    void resize(u32 width, u32 height, u32 format, u32 filter = eBILINEAR, const bool hasMipmap = false, u32 type = eNONE);

    const std::string& getpath() const;

    bool isvalid() const;

    const u32& getwidth() const;

    const u32& getheight() const;

    const u32& getinternalformat() const;

    GLuint getID();

    u32 m_internalformat;
protected:
    std::string path;

    GLuint m_texture;

    u32 m_width;
    u32 m_height;

    bool m_hasMipmap;

    u32 m_format;

    u32 m_filter;

    u32 m_type;

    u32 m_usage;

    u32 m_textureType;

    u32 m_wrapMode;

    friend class Resources;
    friend class Cubemap;
};
