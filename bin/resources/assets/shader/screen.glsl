#define_type postprocess

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

