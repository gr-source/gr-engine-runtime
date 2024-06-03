#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 lPos;

void main() {
    lPos = aPos;

    gl_Position = projection * view * vec4(lPos, 1.0);
}
