#include "screenbuffer.h"

#include "../settings.h"
#include "../screen.h"

ScreenBuffer& ScreenBuffer::getInstance() {
	static ScreenBuffer pScreenBuffer(Settings::GameGrapicQiality.x, Settings::GameGrapicQiality.y);
	return pScreenBuffer;
}

ScreenBuffer::ScreenBuffer(u32 width, u32 height) : m_width(width), m_height(height), samples(4), m_renderbuffer(std::make_unique<Renderbuffer>()), m_textureSample(std::make_unique<Texture>("default", Texture::eTEXTURE2D_MULTISAMPLE)), m_texture(std::make_unique<Texture>()) {
	m_fbo[0] = std::make_unique<Framebuffer>();
	m_fbo[1] = std::make_unique<Framebuffer>();

	m_textureSample->resize(m_width, m_height, Texture::eRGB, Texture::eRGB, Texture::eBILINEAR, Texture::eBYTE, false);
	m_textureSample->apply();

	m_renderbuffer->bind();
	m_renderbuffer->bufferStorageSampler(Renderbuffer::eDEPTH_COMPONENT, samples, width, height);
	m_renderbuffer->unbind();

	m_fbo[FRAMEFBO]->bind();
	m_fbo[FRAMEFBO]->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT0, Framebuffer::eTextureSample2D, m_textureSample.get());
	m_fbo[FRAMEFBO]->SetRenderbuffer(Framebuffer::eDEPTH_ATTACHMENT, m_renderbuffer.get());
	m_fbo[FRAMEFBO]->unbind();

	/*********** POSPROCESSING ***********/
	m_texture->resize(m_width, m_height, Texture::eRGB, Texture::eRGB, Texture::eBILINEAR, Texture::eBYTE, false);
	m_texture->apply();

	m_fbo[POSFBO]->bind();
	m_fbo[POSFBO]->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT0, Framebuffer::eTexture2D, m_texture.get());
	m_fbo[POSFBO]->unbind();
}

ScreenBuffer::~ScreenBuffer() {
}

void ScreenBuffer::bind() {
	GSetViewport(m_width, m_height);
    m_fbo[FRAMEFBO]->bind();
}

void ScreenBuffer::unbind() {
    m_fbo[FRAMEFBO]->unbind();
}

void ScreenBuffer::swap() {
	m_fbo[FRAMEFBO]->read();
	m_fbo[POSFBO]->draw();
	GSetBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void ScreenBuffer::resize(const Vector2& scale) {
    m_width = scale.x;
    m_height = scale.y;

	m_textureSample->resize(m_width, m_height, Texture::eRGBA32F, Texture::eRGBA, Texture::eBILINEAR, Texture::eFLOAT, false);
	m_textureSample->apply();

	m_renderbuffer->bind();
	m_renderbuffer->bufferStorageSampler(Renderbuffer::eDEPTH_COMPONENT32, samples, m_width, m_height);
	m_renderbuffer->unbind();

	m_texture->resize(m_width, m_height, Texture::eRGBA32F, Texture::eRGBA, Texture::eBILINEAR, Texture::eFLOAT, false);
	m_texture->apply();
}

void ScreenBuffer::setsample(u32 sample) {
	samples = sample;
}

const u32& ScreenBuffer::getsample() const {
	return samples;
}

Texture *ScreenBuffer::GetTexture() {
    return m_texture.get();
}

