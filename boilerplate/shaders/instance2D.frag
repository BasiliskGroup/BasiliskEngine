#version 330 core

#include "include/material.glsl"
#include "include/texture.glsl"

in vec2 uv;
flat in Material material;
out vec4 fragColor;

void main() {
    vec4 textureColor = getTextureValue(material, uv);
    if (textureColor.a <= 0.01) { discard; }
    fragColor = textureColor;
}