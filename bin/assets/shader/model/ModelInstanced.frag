#version 330 core

out vec4 FragColor;

#define PI 3.14159265358979323846

#define MAX_MATERIAL 50
#define MAX_TEXTURE 10
#define MAX_LIGHT 10

#define TEXTURE_ALBEDO 0
#define TEXTURE_NORMAL 1
#define TEXTURE_ROUGHNESS 2
#define TEXTURE_AO 3
#define TEXTURE_METALLIC 4

#define LIGHT_DIRECTION 1.0
#define LIGHT_POINT 2.0
#define LIGHT_SPOT 3.0

in vec3 m_normal;
in vec3 m_fragPos;
in vec2 m_textCoord;

flat in int m_materialIndex;

struct Material {
    vec4 albedo;
    vec3 params; // roughness, metallic, ao
};

uniform Material materials[MAX_MATERIAL];

uniform sampler2D textures[MAX_TEXTURE];
uniform bool isactive[MAX_TEXTURE];

void main() {
    vec3 albedo     = materials[m_materialIndex].albedo.xyz;

    if (isactive[TEXTURE_ALBEDO]) {
        albedo = texture(textures[TEXTURE_ALBEDO], m_textCoord).xyz;
    }

    FragColor = vec4(albedo, 1.0);
}
