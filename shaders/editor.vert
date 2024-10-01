#version 330 core


layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

out vec2 uv;
out vec2 viewportUV;

uniform vec4 viewportRect;


void main()
{
    gl_Position = vec4(in_position.xy, 0.0, 1.0); 
    uv = in_uv;
    viewportUV = vec2((in_uv.x - viewportRect[0]) / viewportRect[2], (in_uv.y - viewportRect[1]) / viewportRect[3]);
}  