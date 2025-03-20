#version 330 core

layout (location = 0) out vec4 fragColor;

in vec2 uv;

uniform sampler2D screenTexture;


void main()
{ 
    vec4 color = texture(screenTexture, uv);
    fragColor = color;
}