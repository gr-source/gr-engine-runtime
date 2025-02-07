#shader_type vertex

#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCood;
layout (location = 3) in int in_index;
layout (location = 4) in int in_entityID;

flat out int u_entityID;
flat out int u_index;
out vec3 u_fragPos;
out vec3 u_normal;
out vec2 u_texCoord;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 ViewDirection;

void main() {
    vec4 worldPos = u_model * vec4(in_pos, 1.0);
    vec4 worldNorm = u_model * vec4(in_normal, 1.0);

    u_fragPos = worldPos.xyz;

    u_index = in_index;
    
    u_entityID = in_entityID;

    u_texCoord = in_texCood;

    u_normal = mat3(u_model) * in_normal;

    ViewDirection  = worldPos.xyz;

    gl_Position = u_projection * u_view * worldPos;
}


#shader_type fragment

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
out int EntityID;

flat in int u_index;
flat in int u_entityID;
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
    mat4 model;
};

uniform LightComponent m_light[10];
uniform Material m_material[10];
uniform sampler2D m_texture[10];
uniform bool m_activate[10];
uniform int m_lightcount;
uniform vec3 m_eye;
// uniform sampler2D m_shadowMap;

/*
float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    projCoords = projCoords * 0.5 + 0.5;
    
    float closestDepth = texture(m_shadowMap, projCoords.xy).r;
    
    float currentDepth = projCoords.z;
        
	float bias = 0.005;
	float shadow = 0.0; // currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
	vec2 texelSize = 1.0 / textureSize(m_shadowMap, 0);
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
		    float pcfDepth = texture(m_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
		    shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}
*/

vec3 CreatePointLight(LightComponent light, vec3 V, vec3 N, vec3 F0, vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao) {
    // Calculate the light direction and half vector
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);

    // Compute dot products
    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.001, dot(N, V));
    float NdotH = max(0.001, dot(N, H));

    // Attenuation
    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * d + light.attenuation.z * d * d);

    // Radiance
    vec3 radiance = light.color.xyz * attenuation;

    // Shadow calculation (assuming a point light's shadow map is set up)
    // vec4 fragPosLightSpace = light.model * vec4(fragPos, 1.0);
    // float shadow = ShadowCalculation(fragPosLightSpace);

    // Cook-Torrance BRDF calculations
    float NDF = DistributionGGX(NdotH, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Reflection coefficients
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Specular reflection
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    // Final lighting result considering shadow
    return ((kD * albedo / PI + specular) * radiance * NdotL) /* (1.0 - shadow)*/ ; // Apply shadow attenuation
}

vec3 CreateSpotLight(LightComponent light, vec3 V, vec3 N, vec3 F0, vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao) {
    // Light direction and half vector
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);

    // Dot products
    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.001, dot(N, V));
    float NdotH = max(0.001, dot(N, H));

    // Spotlight cone intensity
    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.params.x - light.params.y;
    float intensity = clamp((theta - light.params.y) / epsilon, 0.0, 1.0);

    // Attenuation
    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * d + light.attenuation.z * (d * d));

    // Radiance
    vec3 radiance = light.color.xyz * attenuation * intensity;

    // Shadow calculation
    // vec4 fragPosLightSpace = light.model * vec4(fragPos, 1.0);
    // float shadow = ShadowCalculation(fragPosLightSpace);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(NdotH, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Reflection coefficients
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Specular reflection
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Final lighting result considering shadow
    return ((kD * albedo / PI + specular) * radiance * NdotL) /* (1.0 - shadow)*/; // Apply shadow attenuation
}


vec3 CreateDirectLight(LightComponent light, vec3 fragPos, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic) {
    // A direção da luz deve ser normalizada e não deve usar a posição da luz.
    vec3 L = normalize(-light.direction); // A luz direcional vem de uma direção constante, por isso é -light.direction

    // Calculando o vetor meio (half-vector) entre a direção da luz e o vetor visual
    vec3 H = normalize(L + V);  // Meio vetor (half-vector)
    vec3 R = reflect(-V, N);    // Vetor refletido (para efeitos especulares)

    // Calcula os produtos de ponto entre vetores
    float NdotL = max(1e-5, dot(N, L));   // Produto escalar normal - direção da luz
    float NdotV = max(1e-5, dot(N, V));   // Produto escalar normal - vetor visual
    float HdotV = max(1e-5, dot(H, V));   // Produto escalar meio-vetor - vetor visual
    float NdotH = max(1e-5, dot(N, H));   // Produto escalar normal - meio-vetor

    // Aproximação de Fresnel Schlick
    vec3 F = fresnelSchlick(HdotV, F0);

    // Função de distribuição (NDF - Normal Distribution Function)
    float D = DistributionGGX(NdotH, roughness);

    // Termo de geometria (G - Geometry term)
    float G = GeometrySmith(N, V, L, roughness);

    // Cálculo do termo especular
    vec3 numerator = D * G * F;
    float denominator = max(4.0 * NdotV * NdotL, 1e-5);
    vec3 specular = numerator / denominator;

    // Cálculo do termo difuso
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;  // Difuso é 1 menos a reflexão especular
    
    // Calcular a profundidade do fragmento no espaço da luz para sombra
    
    // Atenuação do metal
    kD *= 1.0 - metallic;

    // Calcular a contribuição difusa e especular da luz
    // Radiância da luz (cor da luz multiplicada pela intensidade)
    vec3 radiance = light.color.rgb;
    
    // vec4 fragPosLightSpace = light.model * vec4(fragPos, 1.0); // Transformando a posição do fragmento no espaço da luz
    // float shadow = ShadowCalculation(fragPosLightSpace);  // Verificando a sombra para este fragmento

    
    // O resultado final de iluminação leva em conta a sombra (atenuando a luz)
    return ((kD * albedo / PI + specular) * radiance * NdotL /* (1.0 - shadow)*/); // Aplicando a atenuação de sombra
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
    if (u_index == 0 && m_activate[0]) {
    	albedo = texture(m_texture[0], u_texCoord);
    } else if (u_index == 1 && m_activate[1]) {
    	albedo = texture(m_texture[1], u_texCoord);
    } else if (u_index == 2 && m_activate[2]) {
    	albedo = texture(m_texture[2], u_texCoord);
    } else if (u_index == 3 && m_activate[3]) {
    	albedo = texture(m_texture[3], u_texCoord);
    } else if (u_index == 4 && m_activate[4]) {
    	albedo = texture(m_texture[4], u_texCoord);
    }  else if (u_index == 5 && m_activate[5]) {
    	albedo = texture(m_texture[5], u_texCoord);
    }  else if (u_index == 6 && m_activate[6]) {
    	albedo = texture(m_texture[6], u_texCoord);
    }  else if (u_index == 7 && m_activate[7]) {
    	albedo = texture(m_texture[7], u_texCoord);
    }  else if (u_index == 8 && m_activate[8]) {
    	albedo = texture(m_texture[8], u_texCoord);
    }  else if (u_index == 9 && m_activate[9]) {
    	albedo = texture(m_texture[9], u_texCoord);
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
    EntityID = u_entityID;
}



