#version 330 core

layout (location = 0) in vec3 position;
layout (location = 0) in vec4 color;

uniform mat4 u_vp;

out vec4 u_color;

void main() {
    u_color = color;

    gl_Position = u_vp * vec4(position, 1.0);
}

