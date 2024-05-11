#version 330 core

out vec4 FragColor;

in vec2 m_textCoord;

uniform sampler2D m_texture;

void main() {
    FragColor = texture(m_texture, m_textCoord);
}

