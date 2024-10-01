#version 330 core

out vec4 fragColor;

in vec2 uv;
in vec2 viewportUV;

uniform sampler2D screenTexture;
uniform sampler2D engineTexture;


void main()
{ 
    vec4 UI = texture(screenTexture, vec2(uv.x, -uv.y));
    vec4 engine = texture(engineTexture, viewportUV);

    // Correct swizzle
    float red = UI.r;
    UI.r = UI.b;
    UI.b = red;

    fragColor = UI + engine * (1.0 - UI.a);
}