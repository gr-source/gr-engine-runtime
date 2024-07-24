#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCood;
layout (location = 3) in int in_index;

flat out int u_index;
out vec3 u_fragPos;
out vec3 u_normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 worldPos = model * vec4(in_pos, 1.0);
    vec4 worldNorm = model * vec4(in_normal, 1.0);

    u_fragPos = worldPos.xyz;

    u_index = in_index;

    u_normal = mat3(model) * in_normal;

    gl_Position = projection * view * worldPos;
}
