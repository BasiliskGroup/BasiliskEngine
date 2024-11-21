#version 330 core

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec3 in_color;

out vec3 color;

void main()
{
    color = in_color / 255.0;
    gl_Position = vec4(in_position.x, -in_position.y, 0.0, 1.0); 
}  