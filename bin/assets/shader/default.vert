#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 s_fragPos;
out vec3 s_normal;
out vec2 s_texCoord;

void main() {
    gl_Position = projection * view * model * vec4(in_pos, 1.0);

    s_fragPos = vec3(model * vec4(in_pos, 1.0));

    s_normal = mat3(transpose(inverse(model))) * in_normal;

    s_texCoord = in_texCoord;
}

