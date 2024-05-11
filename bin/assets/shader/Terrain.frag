#version 330 core

#define TEXTURE_ALBEDO 0
#define TEXTURE_NORMAL 1
#define TEXTURE_ROUGHNESS 2
#define TEXTURE_AO 3
#define TEXTURE_METALLIC 4

#define MAX_TEXTURE 6
#define MAX_MATERIAL 20

#define PI 3.14159265358979323846

struct Light {
    vec3 position;
    vec4 color;
};

in Terrain {
    vec2 textCoord;
    vec3 normal;
    vec3 eye;
    vec3 fragPos;
    mat3 TBN;
} terrain;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getNormalMap(sampler2D normalMap, vec2 textCoord) {
    vec3 normal = texture2D(normalMap, textCoord).xyz;
    normal = normalize(normal * 2.0 - 1.0);
    normal = terrain.TBN* normal;    
    return normal;
}

vec3 BaseColor(vec3 albedo, float roughness, float metallic, float ao, vec3 normal, Light light) {
    /* normalizaçao */
    vec3 V = normalize(terrain.eye - terrain.fragPos);
    vec3 N = normalize(normal);
    vec3 L = normalize(light.position);
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);  

    /* Fresnel Schlick Approximation */
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo, metallic);
    vec3 F  = fresnelSchlick(NdotL, F0);

    float NDF = DistributionGGX(N, H, roughness) * max(dot(N, V), 0.0);       
    float G   = GeometrySmith(N, V, L, roughness);

    /* calculo da especula */
    vec3 numerator    = NDF * G * F;
    float denominator = max(4.0 * max(dot(N, V), 0.0) * NdotL, 0.0001);
    vec3 specular     = numerator / denominator;

    /* cdifusa e especular */
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    
    kD *= 1.0 - metallic;
      
    vec3 Lo = (kD * albedo / PI + specular) * light.color.xyz * NdotL;

    /* cor ambiente */
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    return color;
}

uniform Light light;
uniform sampler2D textures[MAX_TEXTURE];
uniform bool isActive[MAX_TEXTURE];

out vec4 FragColor;

void main() {
    vec4 albedo = vec4(0.8, 0.8, 0.8, 1.0);
    float metallic = 0.0;
    float roughness = 0.32;
    float ao = 1.0;

    vec3 normal = terrain.normal;

    if (isActive[TEXTURE_ALBEDO]) {
        albedo = texture(textures[TEXTURE_ALBEDO], terrain.textCoord);
    }

    if (isActive[TEXTURE_NORMAL]) {
        normal = getNormalMap(textures[TEXTURE_NORMAL], terrain.textCoord);
    }

    if (isActive[TEXTURE_METALLIC]) {
        metallic = texture(textures[TEXTURE_METALLIC], terrain.textCoord).r;
    }

    if (isActive[TEXTURE_ROUGHNESS]) {
        roughness = texture(textures[TEXTURE_ROUGHNESS], terrain.textCoord).r;
    }

    if (isActive[TEXTURE_AO]) {
        ao = texture(textures[TEXTURE_AO], terrain.textCoord).r;
    }

    vec3 final = BaseColor(albedo.xyz, roughness, metallic, ao, normal, light);

    FragColor = vec4(pow(final, vec3(2.2)), 1.0);
}



