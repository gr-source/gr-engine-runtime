#define_type shader2D

#define_raster vertex

void vertex()
{}

#define_raster fragment

uniform sampler2D u_texture;

void fragment()
{
    const float gamma = 2.2;
    const float exposure = 3.5;

    vec3 hdrColor = texture(u_texture, u_texCoord).rgb;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}

/*
#shader_type vertex

#version 330 core

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texCoord;

out vec2 u_texCoord;

uniform mat4 u_projection;
uniform mat4 u_model;

void main() {
    gl_Position = u_projection * u_model * vec4(in_position, 0.0f, 1.0); 

    u_texCoord = in_texCoord;
}

#shader_type fragment

#version 330 core

out vec4 FragColor;
  
in vec2 u_texCoord;

uniform sampler2D u_texture;

void main() {
    vec3 color = texture(u_texture, u_texCoord).rgb;

    // HDR
    color = vec3(1.0) - exp(color * 1.3);

    // Gama Correction
    color = pow(color, vec3(1.0 / 2.2));

    // OutPut Color
    FragColor = vec4(color, 1.0);
    
    // FragColor = texture(u_texture, u_texCoord);
}


*/
