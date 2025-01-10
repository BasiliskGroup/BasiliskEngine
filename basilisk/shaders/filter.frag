#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D screenTexture;


void main()
{
    vec2  direction   =  normalize(vec2(1.0, 1.0));
    float magnitude   =  0.6;
    float redOffset   =  0.009 * magnitude;
    float greenOffset =  0.006 * magnitude;
    float blueOffset  = -0.006 * magnitude;

    float r = texture(screenTexture, uv + direction * redOffset).r;
    float g = texture(screenTexture, uv + direction * greenOffset).g;
    float b = texture(screenTexture, uv + direction * blueOffset).b;

    fragColor = vec4(r, g, b, 1.0);
}