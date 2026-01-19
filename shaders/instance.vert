#version 330 core

#include "include/material.glsl"

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vNormal;


uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform int uMaterialID;

out vec3 position;
out vec2 uv;
out vec3 normal;
flat out Material material;

void main() {
    position = (uModel * vec4(vPosition, 1.0)).xyz;
    uv = vUV;
    normal = (uModel * vec4(vNormal, 0.0)).xyz;
    material = getMaterial(uMaterialID);

    gl_Position = uProjection * uView * uModel * vec4(vPosition, 1.0);
}
