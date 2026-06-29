#define_type vertex

layout(std140, binding = 0) uniform Camera
{
    mat4 u_projection;
    mat4 u_view;
    vec3 u_eye;
};

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec4 in_color;

out vec4 u_color;

void vertex();

void main()
{
    vec4 worldPos = vec4(in_pos.xyz, 1.0);
    u_color = in_color;

    vec4 finalPos =
        u_projection
        * u_view
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
