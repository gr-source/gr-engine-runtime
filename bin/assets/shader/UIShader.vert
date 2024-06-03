#version 330 core

layout (location = 0) in vec2 local;
layout (location = 1) in vec2 uv;

out vec2 textCoord;

uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * model * vec4(local, 0.0, 1.0);

    textCoord = uv;
}

