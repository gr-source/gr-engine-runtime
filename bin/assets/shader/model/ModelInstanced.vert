#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 aTextCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent; 
layout (location = 5) in int materialID;
layout (location = 6) in ivec4 in_indices;
layout (location = 7) in vec4 in_weights;
layout (location = 8) in mat4 in_model;

uniform mat4 projection;
uniform mat4 view;

out vec3 m_normal;
out vec3 m_fragPos;
out vec2 m_textCoord;

flat out int m_materialIndex;

void main() {
    vec4 worldPos = in_model * vec4(in_pos, 1.0);

    vec3 N = normalize(mat3(in_model) * in_normal);

    m_normal = N;

    m_fragPos = worldPos.xyz;

    m_textCoord = aTextCoord;

    m_materialIndex = materialID;

    gl_Position = projection * view * worldPos;
}
