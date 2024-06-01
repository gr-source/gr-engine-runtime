#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 aTextCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent; 
layout (location = 5) in int materialID;
layout (location = 6) in ivec4 in_indices;
layout (location = 7) in vec4 in_weights;

#define MAX_BONES 100

uniform mat4 u_boneMatrices[MAX_BONES];
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 vpLight;
uniform bool isanimation;

out vec3 m_normal;
out vec3 m_fragPos;
out vec2 m_textCoord;
out vec4 m_weights;

flat out int m_materialIndex;

void main() {
    vec3 SkinVertex = vec3(0.0);
    vec3 SkinNormal = vec3(0.0);
    
    if (isanimation) {
        for (int i = 0; i < 4; ++i) {
            if (in_indices[i] >= MAX_BONES) {
                SkinVertex = in_pos;
                break;
            }
            if (in_indices[i] == -1) {
                continue;
            }
            SkinVertex += (u_boneMatrices[in_indices[i]] * vec4(in_pos, 1.0)).xyz * in_weights[i];
            SkinNormal += (mat3(u_boneMatrices[in_indices[i]]) * in_normal) * in_weights[i];
            m_weights += in_weights[i];
        }
    } else {
        SkinVertex = in_pos;
        SkinNormal = in_normal;
    }

    vec4 worldPos = model * vec4(SkinVertex, 1.0);//* vec4(in_pos, 1.0);

    // mat4 normalMatrix = transpose(inverse(model));
    // vec3 T = normalize(vec3(normalMatrix * vec4(aTangent,   0.0)));
    // vec3 B = normalize(vec3(normalMatrix * vec4(aBitangent, 0.0)));
    vec3 N = normalize(mat3(model) * SkinNormal);

    // m_TBN = mat3(T, B, N);

    m_normal = N;

    m_fragPos = worldPos.xyz;

    m_textCoord = aTextCoord;

    m_materialIndex = materialID;

    gl_Position = projection * view * worldPos;
}
