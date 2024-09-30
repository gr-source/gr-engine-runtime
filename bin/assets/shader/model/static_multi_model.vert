#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCood;
layout (location = 3) in int in_index;

flat out int u_index;
out vec3 u_fragPos;
out vec3 u_normal;
out vec2 u_texCoord;

uniform mat4 u_projection;
uniform mat4 u_view;

void main() {
    u_fragPos = in_pos;

    u_index = in_index;

    u_texCoord = in_texCood;

    u_normal = in_normal;

    gl_Position = u_projection * u_view * vec4(in_pos, 1.0);
}
