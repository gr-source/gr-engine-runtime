#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gRoughness;
layout (location = 4) out vec4 gAo;
layout (location = 5) out vec4 gMetallic;

// out vec4 FragColor;

#define MAX_MATERIAL 50
#define MAX_TEXTURE 10

#define TEXTURE_ALBEDO 0
#define TEXTURE_NORMAL 1
#define TEXTURE_ROUGHNESS 2
#define TEXTURE_AO 3
#define TEXTURE_METALLIC 4

in vec2 m_textCoord;
in vec3 m_fragPos;
in vec3 m_normal;
in mat3 m_TBN;

flat in int m_mindex;

struct Material {
    vec4 albedo;
    float roughness;
    float metallic;
    float ao;
};

uniform Material materials[MAX_MATERIAL];
uniform sampler2D textures[MAX_TEXTURE];
uniform bool isactive[MAX_TEXTURE];

// Function to get the normal from the normal map
vec3 getNormalMap(sampler2D normalMap, vec2 textCoord, vec3 fragPos, vec3 normal) {
    vec3 tangentNormal = texture(normalMap, textCoord).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(fragPos);
    vec3 Q2 = dFdy(fragPos);
    vec2 du = dFdx(textCoord);
    vec2 dv = dFdy(textCoord);

    vec3 N = normalize(normal);
    vec3 T = normalize(Q1 * dv.y - Q2 * du.y);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    // Albedo
    if (isactive[TEXTURE_ALBEDO]) {
        gAlbedo = texture(textures[TEXTURE_ALBEDO], m_textCoord);
    } else {
        gAlbedo = materials[m_mindex].albedo;
    }

    // Position
    gPosition = vec4(m_fragPos, 1.0);

    // Normal
    gNormal = vec4(normalize(m_normal), 1.0);
    if (isactive[TEXTURE_NORMAL]) {
        gNormal = vec4(getNormalMap(textures[TEXTURE_NORMAL], m_textCoord, m_fragPos, m_normal), 1.0);
    }

    // Roughness
    if (isactive[TEXTURE_ROUGHNESS]) {
        gRoughness = texture(textures[TEXTURE_ROUGHNESS], m_textCoord);
    } else {
        gRoughness = vec4(materials[m_mindex].roughness, 0.0, 0.0, 1.0);
    }

    // Ambient Occlusion
    if (isactive[TEXTURE_AO]) {
        gAo = texture(textures[TEXTURE_AO], m_textCoord);
    } else {
        gAo = vec4(materials[m_mindex].ao, 0.0, 0.0, 1.0);
    }

    // Metallic
    if (isactive[TEXTURE_METALLIC]) {
        gMetallic = texture(textures[TEXTURE_METALLIC], m_textCoord);
    } else {
        gMetallic = vec4(materials[m_mindex].metallic, 0.0, 0.0, 1.0);
    }
}
