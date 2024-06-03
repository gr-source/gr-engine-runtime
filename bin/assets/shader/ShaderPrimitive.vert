#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 projection;
uniform mat4 view;

out vec4 m_color;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);

    m_color = aColor;
}