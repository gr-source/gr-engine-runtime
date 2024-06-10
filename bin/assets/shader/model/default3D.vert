#version 330 core

layout (location = 0) in vec3 in_pos;

out vec3 m_fragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(in_pos, 1.0);

    m_fragPos = in_pos;
}
