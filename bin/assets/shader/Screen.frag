#version 330 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;

    color = vec3(1.0) - exp(color * 1.5);

    FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
    // FragColor = vec4(color, 1.0);
}