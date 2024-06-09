#version 330 core

out vec4 FragColor;

in vec3 m_fragPos;

void main() {
    FragColor = vec4(m_fragPos, 1.0);
}