#version 330 core

#include "include/material.glsl"
#include "include/texture.glsl"

in vec3 position;
in vec2 uv;
in vec3 normal;
flat in Material material;

uniform sampler2D uTexture;
uniform vec3 uCameraPosition;
uniform vec3 uViewDirection;

out vec4 fragColor;

void main() {
    vec3 N = normalize(normal);
    vec3 L = normalize(vec3(.5, 1.0, .25));
    vec3 V = normalize(uCameraPosition - position);
    vec3 H = normalize(L + V);

    float ambient = 0.1;
    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64);
    float brightness = (ambient + diffuse + specular);

    vec4 textureColor = getTextureValue(material, uv);
    fragColor = vec4(brightness * textureColor.rgb, textureColor.a);
} 