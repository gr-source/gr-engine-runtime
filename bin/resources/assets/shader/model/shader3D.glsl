#define_type vertex

layout (location = 0) in vec3 in_pos;

#ifndef ENABLE_LINE
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCoord;
// layout (location = 3) in int in_entityID;

#ifdef ENABLE_ANIMATION
        layout (location = 3) in ivec4 in_indices;
        layout (location = 4) in vec4 in_weight;
#endif

#ifdef USE_INSTANCED

#ifdef ENABLE_ANIMATION
        layout (location = 5) in mat4 u_mat_model;
#else // ENABLE_ANIMATION
        layout (location = 3) in mat4 u_mat_model;
#endif // ENABLE_ANIMATION

#else // USE_INSTANCED
        uniform mat4 u_mat_model;
#endif // USE_INSTANCED

        /*
#ifdef USE_MODEL
#ifdef USE_INSTANCED

#ifndef ENABLE_ANIMATION
            layout (location = 3) in mat4 u_mat_model;
#else // ENABLE_ANIMATION
            layout (location = 5) in mat4 u_mat_model;
#endif // ENABLE_ANIMATION
#else
            uniform mat4 u_mat_model;
#endif
#endif
            */

    flat out int u_entityID;

    out vec2 u_texCoord;
    out vec3 u_fragPos;
    out vec3 u_normal;

#ifdef ENABLE_ANIMATION
    uniform mat4 u_boneMatrices[100];
#endif // ENABLE_ANIMATION

#else // ENABLE_LINE
    layout (location = 1) in vec4 in_color;

    out vec4 u_color;
#endif // ENABLE_LINE

uniform mat4 u_mat_projection;
uniform mat4 u_mat_view;

vec4 u_position;

void vertex();

void main()
{
        vec4 worldpos = vec4(in_pos, 1.0);

#ifndef ENABLE_LINE
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
        worldpos = u_mat_model * SkinVertex;
        u_normal = mat3(u_mat_model) * in_normal;
#else
        worldpos = SkinVertex;
        u_normal = in_normal;
#endif
        u_texCoord = in_texCoord;

        // u_entityID = in_entityID;
        
        u_fragPos = worldpos.xyz;
#else // ENABLE_LINE
        u_color = in_color;
#endif // ENABLE_LINE

    u_position = u_mat_projection * u_mat_view * worldpos;

    vertex();

    gl_Position = u_position;
}

#define_type fragment

out vec4 FragColor;

#ifndef ENABLE_LINE
    out int EntityID;

    flat in int u_entityID;

    in vec2 u_texCoord;
    in vec3 u_fragPos;
    in vec3 u_normal;

    uniform vec3 u_mat_eye;
#else
    in vec4 u_color;
#endif

void fragment();

void main()
{
#ifdef ENABLE_LINE
    FragColor = u_color;
#endif

    fragment();

#ifndef ENABLE_LINE
    EntityID = u_entityID;
#endif
}




