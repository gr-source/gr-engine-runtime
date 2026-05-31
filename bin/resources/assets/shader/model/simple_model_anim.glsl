#define_type shader3D

#define USE_MODEL

#define_raster vertex

#define MAX_BONES 100

uniform mat4 u_boneMatrices[100];

void vertex()
{
    vec4 SkinVertex = vec4(0.0);

    for (int i=0;i<4;i++)
    {
        if (in_indices[i] == -1)
        {
            continue;
        }
        
        if (in_indices[i] >= MAX_BONES)
        {
            SkinVertex = vec4(in_pos.xyz, 1.0);
            break;
        }
        
        SkinVertex += u_boneMatrices[in_indices[i]] * vec4(in_pos.xyz, 1.0) * in_weight[i];
    }

    u_position = u_projection * u_view * u_model * SkinVertex;
}

#define_raster fragment

uniform vec3 color;

void fragment()
{
    FragColor = vec4(color, 1.0);
}



