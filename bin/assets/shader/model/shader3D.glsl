#define_type vertex

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCoord;
layout (location = 3) in int in_entityID;

#ifdef ENABLE_ANIMATION
    layout (location = 4) in ivec4 in_indices;
    layout (location = 5) in vec4 in_weight;
#endif

#ifdef USE_MODEL
    #ifdef USE_INSTANCED
        layout (location = 6) in mat4 u_model;
    #else
        uniform mat4 u_model;
    #endif
#endif

flat out int u_entityID;

uniform mat4 u_projection;
uniform mat4 u_view;

#ifdef ENABLE_ANIMATION
    uniform mat4 u_boneMatrices[100];
#endif

vec4 u_position;

void vertex();

void main()
{
    vec4 SkinVertex = vec4(0.0);
    
#ifdef ENABLE_ANIMATION
    for (int i=0;i<4;i++)
    {
        if (in_indices[i] == -1)
        {
            continue;
        }
        
        if (in_indices[i] >= 100)
        {
            SkinVertex = vec4(in_pos, 1.0);
            break;
        }
        
        SkinVertex += u_boneMatrices[in_indices[i]] * vec4(in_pos, 1.0) * in_weight[i];
    }
#else
    SkinVertex = vec4(in_pos, 1.0);
#endif

    #ifdef USE_MODEL
        u_position = u_projection * u_view * u_model * SkinVertex;
    #else
        u_position = u_projection * u_view * SkinVertex;
    #endif

    vertex();

    gl_Position = u_position;

    u_entityID = in_entityID;
}

#define_type fragment

out vec4 FragColor;
out int EntityID;

flat in int u_entityID;

void fragment();

void main()
{
    fragment();

    EntityID = u_entityID;
}
