#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent; 
layout (location = 5) in int materialID;
layout (location = 6) in ivec4 boneIds;
layout (location = 7) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 m_textCoord;
out vec3 m_fragPos;
out vec3 m_normal;
out mat3 m_TBN;

flat out int m_mindex;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    m_TBN = mat3(T, B, N);

    m_textCoord = aTextCoord;
    
    m_fragPos = worldPos.xyz;

    m_normal = normalMatrix * aNormal;

    m_mindex = materialID;

    gl_Position = projection * view * worldPos;
}

