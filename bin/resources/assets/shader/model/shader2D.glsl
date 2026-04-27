#define_type vertex

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_texCoord;

out vec2 u_texCoord;

vec4 u_position;

void vertex();

uniform mat4 u_projection;
uniform mat4 u_model;

void main()
{
    u_texCoord = in_texCoord;

    u_position = u_projection * u_model * vec4(in_pos, 0.0, 1.0);

    vertex();

    gl_Position = u_position;
}

#define_type fragment

out vec4 FragColor;

in vec2 u_texCoord;

void fragment();

void main()
{
    fragment();
}

