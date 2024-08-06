#version 330 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // HDR
    color = vec3(1.0) - exp(color * 1.3);

    // Gama Correction
    color = pow(color, vec3(1.0 / 2.2));

    // OutPut Color
    FragColor = vec4(color, 1.0);
}