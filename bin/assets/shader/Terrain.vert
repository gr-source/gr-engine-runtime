#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out Terrain {
    vec2 textCoord;
    vec3 normal;
    vec3 eye;
    vec3 fragPos;
    mat3 TBN;
} terrain;

uniform mat4 projection;
uniform mat4 view;

uniform vec3 viewPos;

uniform int lenght;

void main() {
    vec3 T = normalize(aPos * tangent);
    vec3 B = normalize(aPos * bitangent);
    vec3 N = normalize(aPos * normal);

    terrain.TBN = transpose(mat3(T, B, N));

    terrain.textCoord = textCoord * lenght;
    terrain.normal    = normal;
    terrain.fragPos   = aPos;
    terrain.eye   = viewPos;

    gl_Position = projection * view * vec4(aPos, 1.0);
}
