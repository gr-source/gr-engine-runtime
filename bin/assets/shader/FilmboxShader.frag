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

#define LIGHT_DIRECTION 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

in vec3 m_normal;
in vec3 m_fragPos;
in vec2 m_textCoord;

flat in int m_materialIndex;

struct Material {
    vec4 albedo;
    vec3 params; // roughness, metallic, ao
};

struct LightComponent {
    vec3 position;
    vec4 color;
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    int type;
    vec3 direction;
    float angleInnerCone;
    float angleOuterCone;
    mat4 vp;
};

uniform LightComponent m_lights[MAX_LIGHT];
uniform Material materials[MAX_MATERIAL];

uniform sampler2D textures[MAX_TEXTURE];
uniform sampler2D gShadow;

uniform bool isactive[MAX_TEXTURE];

uniform int m_lightcount;

uniform vec3 gEye;

// Function to get the normal from the normal map
vec3 getNormalMap(sampler2D normalMap, vec2 textCoord, vec3 fragPos, vec3 normal);

// pbr settings 
vec3 BaseColor(vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao, vec3 normal);

void main() {
    vec3 albedo     = materials[m_materialIndex].albedo.xyz;
    float roughness = materials[m_materialIndex].params.x;
    float metallic  = materials[m_materialIndex].params.y;
    float ao        = materials[m_materialIndex].params.z;

    vec3 normal     = m_normal;

    
    if (isactive[TEXTURE_ALBEDO]) {
        albedo = texture(textures[TEXTURE_ALBEDO], m_textCoord).xyz;
    }

    if (isactive[TEXTURE_NORMAL]) {
        normal = getNormalMap(textures[TEXTURE_NORMAL], m_textCoord, m_fragPos, m_normal);
    }

    if (isactive[TEXTURE_ROUGHNESS]) {
        roughness = texture(textures[TEXTURE_ROUGHNESS], m_textCoord).r;
    }

    if (isactive[TEXTURE_METALLIC]) {
        metallic = texture(textures[TEXTURE_METALLIC], m_textCoord).r;
    }

    if (isactive[TEXTURE_AO]) {
        ao = texture(textures[TEXTURE_AO], m_textCoord).r;
    }

    vec3 color = BaseColor(m_fragPos, albedo, roughness, metallic, ao, normal);

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2)); 

    FragColor = vec4(color, 1.0);
}

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

float DistributionGGX(float NdotH, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
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

vec3 CreatePointLight(LightComponent light, vec3 V, vec3 N, vec3 F0, vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao) {
    // Calcula o vetor de luz e o vetor de meia direção
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);

    // Calcula os produtos de ponto entre vetores
    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.001, dot(N, V));
    float HdotV = max(0.001, dot(H, V));
    float NdotH = max(0.001, dot(N, H));

    // Calcula a atenuação da luz
    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * d + light.attenuationQuadratic * d * d);

    // Calcula a radiância da luz
    vec3 radiance = light.color.xyz * attenuation;
    
    // Calcula os componentes do modelo de reflexão
    float NDF = DistributionGGX(NdotH, roughness); // Função de distribuição
    float G   = GeometrySmith(N, V, L, roughness); // Fator de geometria
    vec3 F    = fresnelSchlick(HdotV, F0); // Fator de Fresnel
    
    // Calcula os coeficientes de reflexão especular e difuso
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    // Calcula a contribuição especular
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001; // Adicionando uma pequena quantidade para evitar divisão por zero
    vec3 specular     = numerator / denominator;
        
    // Combina os componentes especulares e difusos e aplica à radiância de saída (Lo)
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 CreateSpotLight(LightComponent light, vec3 V, vec3 N, vec3 F0, vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao) {
    // Calculate per-light radiance
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);

    // Calcula os produtos de ponto entre vetores
    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.001, dot(N, V));
    // float HdotV = max(0.001, dot(H, V));
    float NdotH = max(0.001, dot(N, H));

    float theta = dot(L, normalize(-light.direction));

    float intensity = 0.0001;

    float epsilon = (light.angleInnerCone - light.angleOuterCone);
    intensity = clamp((theta - light.angleOuterCone) / epsilon, 0.0, 1.0);

    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * d + light.attenuationQuadratic * (d * d));

    vec3 radiance = light.color.xyz * attenuation * intensity;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(NdotH, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // Ensure denominator is not zero
    vec3 specular     = numerator / denominator;
        
    // Add to outgoing radiance Lo
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

float getShadow(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(gShadow, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(gShadow, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth  ? 1.0 : 0.0;
        }    
    }
    shadow /= 9.0;
    
    if (projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

vec3 CreateDirectLight(LightComponent light, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic) {
    vec3 L = normalize(light.position);
    vec3 H = normalize(L + V);
    vec3 R = reflect(-V, N);

    // Calcula o fator de sombreamento
    float shadow = getShadow(light.vp * vec4(m_fragPos, 1.0));

    // Calcula os produtos de ponto entre vetores
    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.001, dot(N, V));
    float HdotV = max(0.001, dot(H, V));
    float NdotH = max(0.001, dot(N, H));

    // Fresnel Schlick Approximation
    vec3 F  = fresnelSchlick(NdotL, F0);

    // Termo de distribuição
    float NDF = DistributionGGX(NdotH, roughness) * NdotV;       
    // Termo de sombreamento
    float G   = GeometrySmith(N, V, L, roughness);

    vec3 numerator    = NDF * G * F;
    float denominator = max(4.0 * NdotV * NdotL, 0.0001);
    vec3 specular     = numerator / max(denominator, 0.0001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    
    // Atenuação do metal
    kD *= 1.0 - metallic;
    kD *= 1.0 - shadow;

    // Aplica o fator de sombreamento
    return (kD * albedo / PI + specular) * light.color.xyz * (1.0 - shadow) * NdotL;
}

vec3 BaseColor(vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao, vec3 normal) {
    vec3 N = normalize(normal);
    vec3 V = normalize(gEye - fragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Calcula a iluminação
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < m_lightcount; ++i) {
        if (m_lights[i].type == LIGHT_POINT) {
            Lo += CreatePointLight(m_lights[i], V, N, F0, fragPos, albedo, roughness, metallic, ao);
        } else if (m_lights[i].type == LIGHT_SPOT) {
            Lo += CreateSpotLight(m_lights[i], V, N, F0, fragPos, albedo, roughness, metallic, ao);
        } else if (m_lights[i].type == LIGHT_DIRECTION) {
            Lo += CreateDirectLight(m_lights[i], V, N, F0, albedo, roughness, metallic);
        }
    }

    // Calcula a iluminação ambiente
    vec3 ambient = vec3(0.03) * albedo * ao;

    // Combinando a iluminação ambiente
    return ambient + Lo;
}


