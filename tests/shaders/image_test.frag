#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D image;

void main()
{ 
    fragColor = vec4(texture(image, uv).rgb, 1.0);
}