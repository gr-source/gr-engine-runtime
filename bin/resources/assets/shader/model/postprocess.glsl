#define_type vertex

out vec2 u_texCoord;

vec4 u_position;

void vertex();

void main()
{
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    vec2 pos = positions[gl_VertexID];

    u_position = vec4(pos, 0.0, 1.0);

    u_texCoord = pos * 0.5 + 0.5;

    vertex();

    gl_Position = u_position;
}

#define_type fragment

out vec4 FragColor;

in vec2 u_texCoord;

void fragment();

void main()
{
    FragColor = vec4(u_texCoord, 0.0, 1.0);
    fragment();
}

