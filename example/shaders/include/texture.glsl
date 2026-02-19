#include "material.glsl"

struct textArray {
    sampler2DArray array;
};

uniform textArray textureArrays[4];

vec4 getTextureValue(Material material, vec2 uv) {
    return texture(textureArrays[material.albedoArray].array, vec3(uv, material.albedoIndex)); 
}