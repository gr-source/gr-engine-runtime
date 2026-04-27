#shader_type vertex

#version 330 core

layout (location = 0) in vec3 in_pos;

uniform mat4 u_model;
uniform mat4 u_light;

void main() {
    gl_Position = u_light * u_model * vec4(in_pos, 1.0);
}


#shader_type fragment

#version 330 core

void main() {}






