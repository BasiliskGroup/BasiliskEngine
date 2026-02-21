#version 330 core

in vec2 uv;

uniform sampler2D uTexture;
uniform vec2 minUV;
uniform vec2 maxUV;

out vec4 fragColor;

void main() {
    vec2 selectionUV = mix(minUV, maxUV, uv);
    fragColor = texture(uTexture, -selectionUV);
}