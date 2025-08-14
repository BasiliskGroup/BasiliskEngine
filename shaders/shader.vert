#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out vec2 uv;
out vec3 normal;


void main() {
    normal = in_normal;
    uv = in_uv;
    gl_Position = projection * view * model * vec4(in_position, 1.0);
}
