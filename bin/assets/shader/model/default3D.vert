#version 330 core

layout (location = 0) in vec3 in_pos;
// layout (location = 1) in mat4 in_model;

out vec3 m_fragPos;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(in_pos, 1.0);

    m_fragPos = in_pos;
}
