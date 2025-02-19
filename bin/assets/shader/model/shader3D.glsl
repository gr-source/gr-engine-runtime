#define_type vertex

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCoord;
layout (location = 3) in int in_entityID;

#ifdef USE_MODEL
#ifdef USE_INSTANCED
layout (location = 4) in mat4 u_model;
#else
uniform mat4 u_model;
#endif // USE_INSTANCED
#endif // USE_MODEL

flat out int u_entityID;

uniform mat4 u_projection;
uniform mat4 u_view;

vec4 u_position;

void vertex();

void main()
{
    #ifdef USE_MODEL
    u_position = u_projection * u_view * u_model * vec4(in_pos, 1.0);
    #else
    u_position = u_projection * u_view * vec4(in_pos, 1.0);
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
