#include "script/light.hpp"

#include "script/transform.hpp"

#include "core/stream.hpp"

std::vector<Light *> Light::m_lights;

Light::Light(std::uint8_t type, const Color& color, float aConstant, float aLinear, float aQuadract, float inneCone, float outerCone) : m_type(static_cast<int>(type)), m_color(color), m_attenuationConstant(aConstant), m_attenuationLinear(aLinear), m_attenuationQuadratic(aQuadract), m_transform(nullptr), m_angleInnerCone(inneCone), m_angleOuterCone(outerCone) {}

Light::~Light() {
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), this), m_lights.end());
}

void Light::Init() {
    m_transform = &entity->getComponent<Transform>();
    
    m_lights.push_back(this);
}

void Light::setColor(const Color &color) {
    m_color = color;
}

void Light::Enable() {
    m_lights.push_back(this);
}

void Light::Disable() {
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), this), m_lights.end());
}

bool Light::Serialize(Stream *stream) {
    stream->write(reinterpret_cast<const char *>(&m_type), sizeof(int));
    stream->write(reinterpret_cast<const char *>(&m_color), sizeof(Color));
    stream->write(reinterpret_cast<const char *>(&m_attenuationConstant), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_attenuationLinear), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_attenuationQuadratic), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_angleInnerCone), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_angleOuterCone), sizeof(float));
    return true;
}

bool Light::Deserialize(Stream *stream) {
    stream->read(reinterpret_cast<char *>(&m_type), sizeof(int));
    stream->read(reinterpret_cast<char *>(&m_color), sizeof(Color));
    stream->read(reinterpret_cast<char *>(&m_attenuationConstant), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_attenuationLinear), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_attenuationQuadratic), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_angleInnerCone), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_angleOuterCone), sizeof(float));
    return true;
}
