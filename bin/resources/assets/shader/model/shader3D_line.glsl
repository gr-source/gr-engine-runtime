#define_type vertex

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec4 in_color;

out vec4 u_color;

uniform mat4 u_mat_projection;
uniform mat4 u_mat_view;

void vertex();

void main()
{
    vec4 worldPos = vec4(in_pos, 1.0);
    u_color = in_color;

    vec4 finalPos =
        u_mat_projection
        * u_mat_view
        * worldPos;

    vertex();

    gl_Position = finalPos;
}

#define_type fragment

out vec4 FragColor;

in vec4 u_color;

void fragment();

void main()
{
    FragColor = u_color;

    fragment();
}
