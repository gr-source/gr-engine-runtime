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
uniform mat4 u_model;

void main() {
    vec4 worldPos = u_model * vec4(in_pos, 1.0);
    vec4 worldNorm = u_model * vec4(in_normal, 1.0);

    u_fragPos = worldPos.xyz;

    u_index = in_index;

    u_texCoord = in_texCood;

    u_normal = mat3(u_model) * in_normal;

    gl_Position = u_projection * u_view * worldPos;
}
