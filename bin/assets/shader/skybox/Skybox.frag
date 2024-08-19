#version 330 core

/* cor de saida */
out vec4 FragColor;

/* variavel recebida de vert */
in vec3 lPos;

uniform sampler2D u_texture;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(lPos));
    vec3 color = texture(u_texture, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
