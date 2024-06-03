#version 330 core

#define MAX_MATERIAL 20
#define PI 3.14159265358979323846

struct Material {
    vec4 albedo;
    float metallic;
    float roughness;
    float ao;
};

struct DirLight {
    vec3 direction;
  
    float ambientStrength;
    float specularStrength;
    vec4 color;
};

in ShaderMain {
    vec3 m_normal;
    vec3 m_fragPos;
    vec3 m_viewPos;
    vec2 m_TexCoord;
    float  m_id;
    mat3 TBN;
} shaderMain;

uniform Material material[MAX_MATERIAL];
uniform DirLight dirLight;
uniform bool IsGamaCorrect;
uniform sampler2D textss;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

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

out vec4 FragColor;

void main() {

    /* normalizaçao */
    vec3 V = normalize(shaderMain.m_viewPos - shaderMain.m_fragPos);
    vec3 N = normalize(shaderMain.m_normal);
    vec3 L = normalize(dirLight.direction);

    /* calculo do half */
    vec3 H = normalize(V + L);

    /* Fresnel Schlick Approximation */
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, material[int(shaderMain.m_id)].albedo.xyz, material[int(shaderMain.m_id)].metallic);
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, material[int(shaderMain.m_id)].roughness);       
    float G   = GeometrySmith(N, V, L, material[int(shaderMain.m_id)].roughness);

    /* calculo da especula */
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
    vec3 specular     = numerator / denominator;

    /* cdifusa e especular */
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    
    kD *= 1.0 - material[int(shaderMain.m_id)].metallic;

    float NdotL = max(dot(N, L), 0.0);        
    vec3 Lo = (kD * material[int(shaderMain.m_id)].albedo.xyz / PI + specular) * 100 * NdotL;

    /* cor ambiente */
    vec3 ambient = dirLight.color.xyz * material[int(shaderMain.m_id)].albedo.xyz * material[int(shaderMain.m_id)].ao;
    vec3 color   = ambient + Lo;

    color = color / (color + vec3(1.0));

    FragColor = vec4(color, 1.0);
    //FragColor = texture(textss, shaderMain.m_TexCoord);
}


