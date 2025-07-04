#define_type shader3D

#define_raster vertex

out vec3 u_texCoord;

void vertex()
{
	u_texCoord = in_pos;

	mat4 rotView = mat4(mat3(u_view));
	vec4 clipPos = u_projection * rotView * vec4(in_pos, 1.0);

	u_position = clipPos.xyww;
}

#define_raster fragment

in vec3 u_texCoord;

uniform samplerCube u_texture;

void fragment()
{
	vec3 envColor = texture(u_texture, u_texCoord).rgb;
    
    // HDR tonemap and gamma correct
    // envColor = envColor / (envColor + vec3(1.0));
    // envColor = pow(envColor, vec3(1.0/2.2));
    
    FragColor = vec4(envColor, 1.0);
}


/*
void main() {
    vec3 envColor = texture(u_texture, WorldPos).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    // envColor = pow(envColor, vec3(1.0/2.2));
    
    FragColor = vec4(envColor, 1.0);
}
*/



