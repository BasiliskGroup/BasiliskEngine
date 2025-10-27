#version 330 core

in vec3 normal;
in vec2 uv;

uniform sampler2D uTexture;

out vec4 fragColor;

void main() {
    vec3 globalLight = normalize(vec3(.5, 1, .25));
    float brightness = (dot(normal, globalLight) + 1) / 2;

    fragColor = brightness * texture(uTexture, uv);
} 