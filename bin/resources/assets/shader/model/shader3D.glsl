#define_type vertex

layout (location = 0) in vec3 in_pos;

layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCoord;
// layout (location = 3) in int in_entityID;

#ifdef ENABLE_ANIMATION
layout (location = 3) in ivec4 in_indices;
layout (location = 4) in vec4 in_weight;
#endif // ENABLE_ANIMATION

#ifdef USE_INSTANCED

    #ifdef ENABLE_ANIMATION
    layout (location = 5) in mat4 u_mat_model;
    #else
    layout (location = 3) in mat4 u_mat_model;
    #endif

#else // USE_INSTANCED
uniform mat4 u_mat_model;
#endif // USE_INSTANCED

flat out int u_entityID;

out vec2 u_texCoord;
out vec3 u_fragPos;
out vec3 u_normal;

#ifdef ENABLE_ANIMATION
uniform mat4 u_boneMatrices[100];
#endif // ENABLE_ANIMATION

uniform mat4 u_mat_projection;
uniform mat4 u_mat_view;

void vertex();

void main()
{
    vec4 localPos = vec4(in_pos, 1.0);

    // =========================
    // Skinning
    // =========================

    #ifdef ENABLE_ANIMATION

    vec4 SkinVertex = vec4(0.0);

    for (int i = 0; i < 4; i++)
    {
        if (in_indices[i] < 0)
            continue;

        if (in_indices[i] >= 100)
            continue;

        SkinVertex +=
            u_boneMatrices[in_indices[i]]
            * localPos
            * in_weight[i];
    }

    // fallback
    if (SkinVertex.w == 0.0)
        SkinVertex = localPos;

    #else // ENABLE_ANIMATION

    vec4 SkinVertex = localPos;

    #endif // ENABLE_ANIMATION

    // =========================
    // Transform
    // =========================

    vec4 worldPos = u_mat_model * SkinVertex;

    u_fragPos = worldPos.xyz;

    // normal matrix correta
    mat3 normalMatrix = transpose(inverse(mat3(u_mat_model)));
    u_normal = normalize(normalMatrix * in_normal);

    u_texCoord = in_texCoord;

    // u_entityID = in_entityID;

    vec4 finalPos =
        u_mat_projection
        * u_mat_view
        * worldPos;

    vertex();

    gl_Position = finalPos;
}

#define_type fragment

out vec4 FragColor;

out int EntityID;

flat in int u_entityID;

in vec2 u_texCoord;
in vec3 u_fragPos;
in vec3 u_normal;

uniform vec3 u_mat_eye;

void fragment();

void main()
{
    fragment();
}


