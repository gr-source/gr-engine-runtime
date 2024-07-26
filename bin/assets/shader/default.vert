#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_nor;
layout (location = 2) in vec2 in_tex;
layout (location = 3) in int in_midx;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(in_pos, 1.0);
}



