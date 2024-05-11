/*#version 330 core

#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

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
uniform vec3 eyePosition;

uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool animated;

out ShaderMain {
    vec3 m_normal;
    vec3 m_fragPos;
    vec3 eyePosition;
    vec2 m_TexCoord;
    float mID;
    mat3 TBN;
} shaderMain;

void main() {
    vec4 totalPosition = vec4(0.0f);
    
    if (animated) {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
            if (boneIds[i] >= MAX_BONES) {
                totalPosition = vec4(aPos, 1.0f);
                break;
            }

            if (boneIds[i] == -1) {
                continue;
            }

            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0);
            totalPosition += localPosition * weights[i];
        }
    } else {
        totalPosition = vec4(aPos, 1.0f);
    }

    vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));

    shaderMain.TBN = transpose(mat3(T, B, N));
    
    shaderMain.mID = float(materialID);

    shaderMain.eyePosition = eyePosition;
    
    shaderMain.m_TexCoord = aTextCoord;
    
    shaderMain.m_fragPos = vec3(model * totalPosition);

    shaderMain.m_normal = transpose(inverse(mat3(model))) * aNormal;

    gl_Position =  projection * view * model * totalPosition;
}
*/

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

out vec3 m_normal;
out vec3 m_fragPos;
out vec2 m_textCoord;
out vec4 m_fragPosLight;

flat out int m_materialIndex;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    // m_TBN = mat3(T, B, N);

    m_normal = N;

    m_fragPos = worldPos.xyz;

    m_textCoord = aTextCoord;

    m_materialIndex = materialID;

    m_fragPosLight = vpLight * worldPos;

    gl_Position = projection * view * worldPos;
}
