#define_type shader3D

#define_raster vertex

out vec3 u_fragPos;

void vertex()
{
	u_fragPos = in_pos;

    u_position = u_projection * u_view * vec4(in_pos, 1.0);
}

#define_raster fragment

in vec3 u_fragPos;

uniform sampler2D u_texture;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void fragment()
{
	vec2 uv = SampleSphericalMap(normalize(u_fragPos));
    vec3 color = texture(u_texture, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}






