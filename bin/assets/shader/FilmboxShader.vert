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
uniform mat4 vpLight;

out mat3 m_TBN;
out vec3 m_normal;
out vec3 m_fragPos;
out vec2 m_textCoord;
out vec4 m_fragPosLight;

flat out int m_materialIndex;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);

    mat4 normalMatrix = transpose(inverse(model));
    vec3 T = normalize(vec3(normalMatrix * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(normalMatrix * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(normalMatrix * vec4(aNormal,    0.0)));

    m_TBN = mat3(T, B, N);

    m_normal = N;

    m_fragPos = worldPos.xyz;

    m_textCoord = aTextCoord;

    m_materialIndex = materialID;

    m_fragPosLight = vpLight * worldPos;

    gl_Position = projection * view * worldPos;
}
