#version 330 core

layout (location = 0) out vec4 fragColor;

in vec2 uv;
uniform sampler2D depthTexture;

void main() {
    fragColor = vec4(vec3(20 * (1 - texture(depthTexture, uv).r)), 1.0);
} 