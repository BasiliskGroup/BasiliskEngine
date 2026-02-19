#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;

#include "include/material.glsl"

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform int uMaterialID;

out vec2 uv;
flat out Material material;

void main() {
    uv = vUV;
    material = getMaterial(uMaterialID);
    gl_Position = uProjection * uView * uModel * vec4(vPosition, 1.0);
}
