#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;

out vec2 m_textCoord;

uniform mat4 vp;
uniform mat4 model;

void main() {
    m_textCoord = aUv;

    gl_Position = vp * model * vec4(aPos, 1.0);
}
