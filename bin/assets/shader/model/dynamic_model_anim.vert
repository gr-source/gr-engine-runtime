#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCood;
layout (location = 3) in int in_index;
layout (location = 4) in ivec4 in_indices;
layout (location = 5) in vec4 in_weights;

flat out int u_index;
out vec3 u_fragPos;
out vec3 u_normal;
out vec2 u_texCoord;

uniform mat4 u_boneMatrices[100];
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform bool u_animatior;

void main() {
    vec3 SkinVertex = vec3(0.0);
    vec3 SkinNormal = vec3(0.0);

    if (u_animatior) {
        for (int i = 0; i < 4; ++i) {
            if (in_indices[i] >= 100) {
                SkinVertex = in_pos;
                break;
            }
            if (in_indices[i] == -1) {
                continue;
            }
            SkinVertex += (u_boneMatrices[in_indices[i]] * vec4(in_pos, 1.0)).xyz * in_weights[i];
            SkinNormal += (mat3(u_boneMatrices[in_indices[i]]) * in_normal) * in_weights[i];
        }
    } else {
        SkinNormal = in_normal;
        SkinVertex = in_pos;
    }

    vec4 worldPos = u_model * vec4(SkinVertex, 1.0);
    vec4 worldNorm = u_model * vec4(SkinNormal, 1.0);

    u_fragPos = worldPos.xyz;

    u_index = in_index;

    u_texCoord = in_texCood;

    u_normal = mat3(u_model) * SkinNormal;

    gl_Position = u_projection * u_view * worldPos;
}
