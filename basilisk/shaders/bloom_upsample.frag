#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D highTexture;
uniform sampler2D lowTexture;

uniform ivec2 textureSize;

void main()
{ 

    vec2 texelSize = 1.0 / textureSize;

    vec4 color = vec4(0.0);

    // Basic 2x2 upsample (averaging)
    color += texture(lowTexture, clamp(uv + vec2( 1,  1) * texelSize, 0.0, 0.9999));
    color += texture(lowTexture, clamp(uv + vec2( 1,  0) * texelSize, 0.0, 0.9999)) * 2.0;
    color += texture(lowTexture, clamp(uv + vec2( 1, -1) * texelSize, 0.0, 0.9999));

    color += texture(lowTexture, clamp(uv + vec2( 0,  1) * texelSize, 0.0, 0.9999)) * 2.0;
    color += texture(lowTexture, clamp(uv + vec2( 0,  0) * texelSize, 0.0, 0.9999)) * 4.0;
    color += texture(lowTexture, clamp(uv + vec2( 0, -1) * texelSize, 0.0, 0.9999)) * 2.0;

    color += texture(lowTexture, clamp(uv + vec2(-1,  1) * texelSize, 0.0, 0.9999));
    color += texture(lowTexture, clamp(uv + vec2(-1,  0) * texelSize, 0.0, 0.9999)) * 2.0;
    color += texture(lowTexture, clamp(uv + vec2(-1, -1) * texelSize, 0.0, 0.9999));

    fragColor = texture(highTexture, uv) + color / 16.0; // Average the four samples
    
}