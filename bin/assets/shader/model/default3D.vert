#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent; 
layout (location = 5) in int materialID;
layout (location = 6) in ivec4 in_indices;
layout (location = 7) in vec4 in_weights;

out vec2 m_texCoord;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(in_pos, 1.0);

    m_texCoord = in_texCoord;
}
