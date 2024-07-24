#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

/* projection * view */
uniform mat4 pv;

/* model */
uniform mat4 model;

/* out vai para o .frag */
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

void main() {
    TexCoords = uv;

    WorldPos = vec3(model * vec4(position, 1.0));

    Normal = normalMatrix * aNormal;  

    gl_Position = pv * model * vec4(position, 1.0);
}


