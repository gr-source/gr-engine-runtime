#version 330 core

out vec4 FragColor;

in vec4 m_color;

void main() {
    FragColor = m_color;
}