#version 330 core

#define PI 3.14159265358979323846

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

#define LIGHT_DIRECTION 0.0
#define LIGHT_POINT 1.0
#define LIGHT_SPOT 2.0

out vec4 FragColor;

flat in int u_index;
in vec3 u_fragPos;
in vec3 u_normal;
in vec2 u_texCoord;

struct Material {
    vec4 albedo;
    vec3 params; // roughness, metallic, ao
};

struct LightComponent {
    vec3 position;
    vec4 color;
    vec3 attenuation; // (x - constant) (y - linear) (z - quadract)
    vec3 direction;
    vec3 params; // (x - angleInCone) (y - angleOutCone) (z - type)
    mat4 vp;
};

uniform LightComponent m_light[10];
uniform Material m_material[10];
uniform sampler2D m_texture[10];
uniform bool m_activate[10];
uniform int m_lightcount;
uniform vec3 m_eye;

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
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * d + light.attenuation.z * d * d);

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

    float epsilon = light.params.x - light.params.y;
    if (light.params.y > light.params.x) {
        epsilon = light.params.y - light.params.x;
    }
    intensity = clamp((theta - light.params.y) / epsilon, 0.0, 1.0);

    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * d + light.attenuation.z * (d * d));

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

vec3 CreateDirectLight(LightComponent light, vec3 fragPos, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic) {
    // A direção da luz deve ser normalizada e não deve usar a posição da luz.
    vec3 L = normalize(-light.direction); // A luz direcional vem de uma direção constante, por isso é -light.direction

    vec3 H = normalize(L + V);  // Vetor de meio (half-vector)
    vec3 R = reflect(-V, N);    // Vetor refletido (para efeitos especulares)

    // Calcula os produtos de ponto entre vetores
    float NdotL = max(1e-5, dot(N, L));   // Produto escalar normal - direção da luz
    float NdotV = max(1e-5, dot(N, V));   // Produto escalar normal - vetor visual
    float HdotV = max(1e-5, dot(H, V));   // Produto escalar meio-vetor - vetor visual
    float NdotH = max(1e-5, dot(N, H));   // Produto escalar normal - meio-vetor

    // Fresnel Schlick Approximation
    vec3 F = fresnelSchlick(HdotV, F0);

    // Termo de distribuição (NDF - Normal Distribution Function)
    float D = DistributionGGX(NdotH, roughness);

    // Termo de sombreamento (G - Geometry term)
    float G = GeometrySmith(N, V, L, roughness);

    // Cálculo do termo especular
    vec3 numerator = D * G * F;
    float denominator = max(4.0 * NdotV * NdotL, 1e-5);
    vec3 specular = numerator / denominator;

    // Cálculo do termo difuso
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    
    // Atenuação do metal
    kD *= 1.0 - metallic;

    // Retorno do resultado da iluminação difusa e especular
    return (kD * albedo / PI + specular) * light.color.rgb * NdotL;
}

vec3 BaseColor(vec3 eye, vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao, vec3 normal) {
    vec3 N = normalize(normal);
    vec3 V = normalize(eye - fragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Inicializa a iluminação total
    vec3 Lo = vec3(0.0);

    // Calcula a iluminação ambiente
    vec3 ambient = vec3(0.03) * albedo * ao; // Considera a cor da luz ambiente como uma constante

    // Itera sobre as luzes
    for (int i = 0; i < m_lightcount; ++i) {
        // Contribuição das luzes
        if (m_light[i].params.z == LIGHT_POINT) {
            Lo += CreatePointLight(m_light[i], V, N, F0, fragPos, albedo, roughness, metallic, ao);
        } else if (m_light[i].params.z == LIGHT_SPOT) {
            Lo += CreateSpotLight(m_light[i], V, N, F0, fragPos, albedo, roughness, metallic, ao);
        } else if (m_light[i].params.z == LIGHT_DIRECTION) {
            Lo += CreateDirectLight(m_light[i], fragPos, V, N, F0, albedo, roughness, metallic);
        }
    }

    // Combinando a iluminação ambiente com a iluminação direta
    return ambient + Lo;
}


void main() {
    vec4 albedo = m_material[u_index].albedo;
    if (m_activate[0]) {
        albedo = texture(m_texture[0], u_texCoord);
    }

    float roughness = m_material[u_index].params.x;
    float metallic  = m_material[u_index].params.y;
    float ao        = m_material[u_index].params.z;

    vec3 color = BaseColor(m_eye, u_fragPos, albedo.rgb, roughness, metallic, ao, u_normal);

    // Linear Color
    color = color / (color + vec3(1.0));

    // Gama Corretion
    color = pow(color, vec3(1.0 / 2.2));

    // Output Color
    FragColor = vec4(color, 1.0);
}


