#version 330 core

#define MAX_BONES 100

layout (location = 0) in vec3 in_pos;
layout (location = 6) in ivec4 in_indices;
layout (location = 7) in vec4 in_weights;

uniform mat4 u_boneMatrices[MAX_BONES];
uniform mat4 model;
uniform mat4 vp;
uniform bool isanimation;

void main() {
    vec3 SkinVertex = vec3(0.0);
    
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
        }
    } else {
        SkinVertex = in_pos;
    }

    vec4 worldPos = model * vec4(SkinVertex, 1.0);
    gl_Position = vp * worldPos;
}
