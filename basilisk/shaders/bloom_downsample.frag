#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D screenTexture;
uniform ivec2 textureSize;

void main()
{ 

    vec2 texelSize = 1.0 / textureSize;

    vec4 color = vec4(0.0);

    // Basic 2x2 downsampling (averaging)
    color += texture(screenTexture, clamp(uv + vec2(-texelSize.x, -texelSize.y), 0.0, 0.9999));
    color += texture(screenTexture, clamp(uv + vec2( texelSize.x, -texelSize.y), 0.0, 0.9999));
    color += texture(screenTexture, clamp(uv + vec2(-texelSize.x,  texelSize.y), 0.0, 0.9999));
    color += texture(screenTexture, clamp(uv + vec2( texelSize.x,  texelSize.y), 0.0, 0.9999));

    fragColor = color / 4.0; // Average the four samples
}