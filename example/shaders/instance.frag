#version 330 core

#include "include/material.glsl"
#include "include/texture.glsl"
#include "include/light.glsl"
#include "include/blinn_phong.glsl"

in vec3 position;
in vec2 uv;
in vec3 normal;
flat in Material material;

uniform sampler2D uTexture;
uniform vec3 uCameraPosition;
uniform vec3 uViewDirection;

out vec4 fragColor;

void main() {
    Tile tile = getTile();
    vec4 textureColor = getTextureValue(material, uv);
    
    vec3 N = normalize(normal);
    vec3 V = normalize(uCameraPosition - position);

    vec3 directionalLightColor = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < uDirectionalLightCount; i++) {
        DirectionalLight directionalLight = directionalLights[i];
        directionalLightColor += calculateDirectionalLight(directionalLight, N, V);
    }
    
    vec3 pointLightColor = vec3(0.0, 0.0, 0.0);
    for (uint i = tile.offset; i < tile.offset + tile.count; i++) {
        PointLight pointLight = getLightByIndex(i);
        pointLightColor += calculatePointLight(pointLight, position, N, V);
    }

    vec3 ambientColor = uAmbientLight;

    fragColor = vec4((directionalLightColor + pointLightColor + ambientColor) * textureColor.rgb, textureColor.a);
} 