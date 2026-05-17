#define_type shader3D

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

struct Light {
    vec3 position;
    vec3 direction;
    vec4 color;
    float range;
    float angle;
    int type; // 0: Direcional, 1: Ponto, 2: Spot
};

uniform Light u_mat_lights[10];
uniform int u_mat_numLights;

vec3 CalculateLight(Light light, vec3 fragPos, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic) {
    vec3 L;
    float attenuation = 1.0;

    if (light.type == 0) {
        L = normalize(-light.direction);
    } else if (light.type == 1) {
        vec3 lightDir = light.position - fragPos;
        float distance = length(lightDir);
        L = normalize(lightDir);
        attenuation = 1.0 / (distance * distance);
    } else if (light.type == 2) {
        vec3 lightDir = light.position - fragPos;
        float distance = length(lightDir);
        L = normalize(lightDir);
        attenuation = 1.0 / (distance * distance);

        float theta = dot(L, normalize(-light.direction));
        if (theta > cos(light.angle)) {
            attenuation *= (theta - cos(light.angle)) / (1 - cos(light.angle));
        } else {
            attenuation = 0.0;
        }
    }

    float NdotL = max(1e-5, dot(N, L));
    float NdotV = max(1e-5, dot(N, V));
    vec3 H = normalize(L + V);
    float NdotH = max(1e-5, dot(N, H));
    float HdotV = max(1e-5, dot(H, V));

    vec3 F = fresnelSchlick(HdotV, F0);

    float D = DistributionGGX(NdotH, roughness);

    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator = D * G * F;
    float denominator = max(4.0 * NdotV * NdotL, 1e-5);
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 radiance = light.color.rgb * attenuation;

    return ((kD * albedo / PI + specular) * radiance * NdotL);
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ambientocclusion;

void fragment()
{
	vec3 N = normalize(u_normal);
	vec3 V = normalize(u_mat_eye - u_fragPos);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    
	for (int i = 0; i < u_mat_numLights; i++)
    {
        Lo += CalculateLight(u_mat_lights[i], u_fragPos, V, N, F0, albedo, roughness, metallic);
    }

    vec3 ambient = vec3(0.1) * albedo * ambientocclusion;

    vec3 color = ambient + Lo;

    float exposure = 2.0;
    color *= exposure;

    color = ACESFilm(color);

    FragColor = vec4(color, 1.0);
    
    /*
    color = color / (color + vec3(1.0));

    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
    */
}



