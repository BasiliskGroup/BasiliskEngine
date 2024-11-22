#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D screenTexture;


void main()
{ 
    fragColor = texture(screenTexture, uv);

    // Correct swizzle
    float red = fragColor.r;
    fragColor.r = fragColor.b;
    fragColor.b = red;
}