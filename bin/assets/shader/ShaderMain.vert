#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textCoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in int id;

uniform mat4 model;
uniform mat4 pv;

uniform vec3 viewPos;

out ShaderMain {
    vec3 m_normal;
    vec3 m_fragPos;
    vec3 m_viewPos;
    vec2 m_TexCoord;
    float  m_id;
    mat3 TBN;
} shaderMain;

void main() {
    vec3 T = normalize(tangent);
    vec3 B = normalize(bitangent);
    vec3 N = normalize(normal);

    shaderMain.TBN = transpose(mat3(T, B, N));

    shaderMain.m_fragPos  = vec3(transpose(inverse(model)) * vec4(position, 1.0));

    shaderMain.m_normal   = mat3(transpose(inverse(model))) * normal;

    shaderMain.m_viewPos  = viewPos;
    shaderMain.m_TexCoord = textCoord;
    shaderMain.m_id       = float(id);

    gl_Position = pv * model * vec4(position, 1.0);
}
