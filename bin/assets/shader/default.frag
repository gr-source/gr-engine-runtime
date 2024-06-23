#version 330 core

out vec4 FragColor;

in vec3 s_fragPos;
in vec3 s_normal;
in vec2 s_texCoord;

uniform vec3 viewPos;

void main() {
    vec3 lightPos = vec3(0.0, 0.0, -5.0);
    vec3 lightColor = vec3(1.0);
    vec3 objectColor = vec3(0.8);

    // Ambiente
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Difusa
    vec3 norm = normalize(s_normal);
    vec3 lightDir = normalize(lightPos - s_fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Especular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - s_fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Cor final do fragmento
    vec3 result = (ambient + diffuse + specular) * objectColor.rgb;
    FragColor = vec4(result, 1.0);
}