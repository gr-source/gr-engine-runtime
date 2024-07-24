#version 330 core

#define PI 3.14159265358979323846

#define LIGHT_DIRECTION 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

#define MAX_LIGHT 100

in vec2 m_textCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughness;
uniform sampler2D gAo;
uniform sampler2D gMetallic;
uniform sampler2D gShadow;

uniform vec3 gEye;

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
uniform int m_lightcount;

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

vec3 CreateDirectLight(LightComponent light, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic) {
    vec3 L = normalize(light.position);
    vec3 H = normalize(L + V);
    vec3 R = reflect(-V, N);

    // Calcula os produtos de ponto entre vetores
    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.001, dot(N, V));
    float HdotV = max(0.001, dot(H, V));
    float NdotH = max(0.001, dot(N, H));

    // Fresnel Schlick Approximation
    vec3 F  = fresnelSchlick(NdotL, F0);

    float NDF = DistributionGGX(NdotH, roughness) * NdotV;       
    float G   = GeometrySmith(N, V, L, roughness);

    vec3 numerator    = NDF * G * F;
    float denominator = max(4.0 * NdotV * NdotL, 0.0001);
    vec3 specular     = numerator / max(denominator, 0.0001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    
    kD *= 1.0 - metallic;
      
    return (kD * albedo / PI + specular) * light.color.xyz * NdotL;
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

void main() {
    vec3 albedo = texture(gAlbedo, m_textCoord).xyz;

    vec3 fragPos = texture(gPosition, m_textCoord).xyz;

    vec3 normal = texture(gNormal, m_textCoord).xyz;

    float roughness = texture(gRoughness, m_textCoord).r;

    float ao = texture(gAo, m_textCoord).r;

    float metallic = texture(gMetallic, m_textCoord).r;
    
    vec3 color = BaseColor(fragPos, albedo, roughness, metallic, ao, normal);

    color = color / (color + vec3(1.0));

    color = vec3(1.0) - exp(color * 2.0);

    FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}

/*
#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0
uniform sampler2D sampler0;
uniform vec2 resolution;

void main(){
   vec2 inverse_resolution=vec2(1.0/resolution.x,1.0/resolution.y);
   vec3 rgbNW = texture2D(sampler0, (gl_FragCoord.xy + vec2(-1.0,-1.0)) * inverse_resolution).xyz;
   vec3 rgbNE = texture2D(sampler0, (gl_FragCoord.xy + vec2(1.0,-1.0)) * inverse_resolution).xyz;
   vec3 rgbSW = texture2D(sampler0, (gl_FragCoord.xy + vec2(-1.0,1.0)) * inverse_resolution).xyz;
   vec3 rgbSE = texture2D(sampler0, (gl_FragCoord.xy + vec2(1.0,1.0)) * inverse_resolution).xyz;
   vec3 rgbM  = texture2D(sampler0,  gl_FragCoord.xy  * inverse_resolution).xyz;
   vec3 luma = vec3(0.299, 0.587, 0.114);
   float lumaNW = dot(rgbNW, luma);
   float lumaNE = dot(rgbNE, luma);
   float lumaSW = dot(rgbSW, luma);
   float lumaSE = dot(rgbSE, luma);
   float lumaM  = dot(rgbM,  luma);
   float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
   float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); 
   vec2 dir;
   dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
   dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
   float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),FXAA_REDUCE_MIN);
   float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
   dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),dir * rcpDirMin)) * inverse_resolution;
   vec3 rgbA = 0.5 * (texture2D(sampler0,   gl_FragCoord.xy  * inverse_resolution + dir * (1.0/3.0 - 0.5)).xyz + texture2D(sampler0,   gl_FragCoord.xy  * inverse_resolution + dir * (2.0/3.0 - 0.5)).xyz);
   vec3 rgbB = rgbA * 0.5 + 0.25 * (texture2D(sampler0,  gl_FragCoord.xy  * inverse_resolution + dir *  - 0.5).xyz + texture2D(sampler0,  gl_FragCoord.xy  * inverse_resolution + dir * 0.5).xyz);
   float lumaB = dot(rgbB, luma);
   if((lumaB < lumaMin) || (lumaB > lumaMax)) {
      gl_FragColor = vec4(rgbA,1.0);
   } else {
      gl_FragColor = vec4(rgbB,1.0);
   }
}
*/






