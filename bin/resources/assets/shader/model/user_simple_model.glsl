#define_type shader3D

#define USE_MODEL

#define_raster vertex

void vertex()
{
}

#define_raster fragment


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

vec3 CreateDirectLight(LightComponent light, vec3 fragPos, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic);
vec3 BaseColor(vec3 eye, vec3 fragPos, vec3 albedo, float roughness, float metallic, float ao, vec3 normal);

uniform vec3 color;

void fragment()
{
    vec4 albedo = vec4(color, 1.0);
    float roughness = 0.5;
    float metallic  = 0.5;
    float ao        = 0.5;

    vec3 color = BaseColor(u_eye, u_fragPos, albedo.rgb, roughness, metallic, ao, u_normal);

    // Linear Color
    color = color / (color + vec3(1.0));

    // Gama Corretion
    color = pow(color, vec3(1.0 / 2.2));

    // Output Color
    FragColor = vec4(color, 1.0);
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
    
    LightComponent light;
    light.position = vec3(0.0);
    light.direction = vec3(0.0, 0.0, 1.0);
    light.color = vec4(1.0, 1.0, 0.0, 1.0);

    Lo = CreateDirectLight(light, fragPos, V, N, F0, albedo, roughness, metallic);
    
    // Combinando a iluminação ambiente com a iluminação direta
    return ambient + Lo;
}
























