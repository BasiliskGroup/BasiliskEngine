#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;


in vec3  in_instance_pos;
in float in_instance_mtl;
in float scale;
in float life;

out vec2 uv;
out mat3 TBN;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform sampler2D materialsTexture;

struct Material {
    vec3  color;
    float roughness;
    float subsurface;
    float sheen;
    float sheenTint;
    float anisotropic;
    float specular;
    float metallicness;
    float specularTint;
    float clearcoat;
    float clearcoatGloss;
    
    int   hasAlbedoMap;
    vec2  albedoMap;
    int   hasNormalMap;
    vec2  normalMap;
    int   hasRoughnessMap;
    vec2  roughnessMap;
    int   hasAoMap;
    vec2  aoMap;
};
flat out Material mtl;

// Function to get the model matrix from node position, rotation, and scale
mat4 getModelMatrix(vec3 pos, float scale) {
    mat4 translation = mat4(
        scale, 0    , 0    , 0,
        0    , scale, 0    , 0,
        0    , 0    , scale, 0,
        pos.x, pos.y, pos.z, 1
    );
    return translation;
}

// Function to get the TBN matrix for normal mapping
mat3 getTBN(mat4 modelMatrix, vec3 normal, vec3 tangent, vec3 bitangent){
    vec3 T = normalize(vec3(modelMatrix * vec4(tangent,   0.0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(normal,    0.0)));
    return mat3(T, B, N);
}

void main() {
    // Set the model matrix
    mat4 modelMatrix = getModelMatrix(in_instance_pos, scale * life);

    // Set out variables
    TBN      = getTBN(modelMatrix, in_normal, in_tangent, in_bitangent);
    uv       = in_uv;

    // Material Data
    int mtl_size = 25;
    int materialID      = int(in_instance_mtl);
    mtl = Material(vec3(0), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, vec2(0), 0, vec2(0), 0, vec2(0), 0, vec2(0));
    mtl.color           = vec3(texelFetch(materialsTexture, ivec2(0, 0 + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 1  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 2  + materialID * mtl_size), 0).r);
    mtl.hasAlbedoMap    = int(texelFetch(materialsTexture,  ivec2(0, 13  + materialID * mtl_size), 0).r);
    mtl.albedoMap       = vec2(texelFetch(materialsTexture, ivec2(0, 14  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 15  + materialID * mtl_size), 0).r);
    mtl.hasNormalMap    = int(texelFetch(materialsTexture,  ivec2(0, 16  + materialID * mtl_size), 0).r);
    mtl.normalMap       = vec2(texelFetch(materialsTexture, ivec2(0, 17  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 18 + materialID * mtl_size), 0).r);    

    // Send position to the frag
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_position, 1.0);
}