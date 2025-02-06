#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

layout (location = 5) in vec3  obj_position;
layout (location = 6) in vec4  obj_rotation;
layout (location = 7) in vec3  obj_scale;
layout (location = 8) in float obj_material;

// Variables passed on to the fragment shader
out vec2 uv;
out vec3 position;
out mat3 TBN;

// Material struct sent to fragment shader
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

// Uniforms
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform sampler2D materialsTexture;

// Function to get the model matrix from node position, rotation, and scale
mat4 getModelMatrix(vec3 pos, vec4 rot, vec3 scl) {
    mat4 translation = mat4(
        1    , 0    , 0    , 0,
        0    , 1    , 0    , 0,
        0    , 0    , 1    , 0,
        pos.x, pos.y, pos.z, 1
    );
    mat4 rotation = mat4(
        1 - 2 * (rot.z * rot.z + rot.w * rot.w), 2 * (rot.y * rot.z - rot.w * rot.x), 2 * (rot.y * rot.w + rot.z * rot.x), 0,
        2 * (rot.y * rot.z + rot.w * rot.x), 1 - 2 * (rot.y * rot.y + rot.w * rot.w), 2 * (rot.z * rot.w - rot.y * rot.x), 0,
        2 * (rot.y * rot.w - rot.z * rot.x), 2 * (rot.z * rot.w + rot.y * rot.x), 1 - 2 * (rot.y * rot.y + rot.z * rot.z), 0,
        0, 0, 0, 1
    );
    mat4 scale = mat4(
        scl.x, 0    , 0    , 0,
        0    , scl.y, 0    , 0,
        0    , 0    , scl.z, 0,
        0    , 0    , 0    , 1
    );
    return translation * rotation * scale;
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
    mat4 modelMatrix = getModelMatrix(obj_position, obj_rotation, obj_scale);

    // Set out variables
    position = (modelMatrix * vec4(in_position, 1.0)).xyz;
    TBN      = getTBN(modelMatrix, in_normal, in_tangent, in_bitangent);
    uv       = in_uv;
    
    // Get the material
    int mtl_size = 25;
    int materialID     = int(obj_material);

    mtl.color          = vec3(texelFetch(materialsTexture, ivec2(0, 0 + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 1  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 2  + materialID * mtl_size), 0).r);
    mtl.roughness      = texelFetch(materialsTexture, ivec2(0, 3 + materialID * mtl_size), 0).r;
    mtl.subsurface     = texelFetch(materialsTexture, ivec2(0, 4 + materialID * mtl_size), 0).r;
    mtl.sheen          = texelFetch(materialsTexture, ivec2(0, 5 + materialID * mtl_size), 0).r;
    mtl.sheenTint      = texelFetch(materialsTexture, ivec2(0, 6 + materialID * mtl_size), 0).r;
    mtl.anisotropic    = texelFetch(materialsTexture, ivec2(0, 7 + materialID * mtl_size), 0).r;
    mtl.specular       = texelFetch(materialsTexture, ivec2(0, 8 + materialID * mtl_size), 0).r;
    mtl.metallicness   = texelFetch(materialsTexture, ivec2(0, 9 + materialID * mtl_size), 0).r;
    mtl.specularTint   = texelFetch(materialsTexture, ivec2(0, 10 + materialID * mtl_size), 0).r;
    mtl.clearcoat      = texelFetch(materialsTexture, ivec2(0, 11 + materialID * mtl_size), 0).r;
    mtl.clearcoatGloss = texelFetch(materialsTexture, ivec2(0, 12 + materialID * mtl_size), 0).r;
    
    mtl.hasAlbedoMap    = int(texelFetch(materialsTexture,  ivec2(0, 13  + materialID * mtl_size), 0).r);
    mtl.albedoMap       = vec2(texelFetch(materialsTexture, ivec2(0, 14  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 15  + materialID * mtl_size), 0).r);
    mtl.hasNormalMap    = int(texelFetch(materialsTexture,  ivec2(0, 16  + materialID * mtl_size), 0).r);
    mtl.normalMap       = vec2(texelFetch(materialsTexture, ivec2(0, 17  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 18 + materialID * mtl_size), 0).r);    
    mtl.hasRoughnessMap = int(texelFetch(materialsTexture,  ivec2(0, 19  + materialID * mtl_size), 0).r);
    mtl.roughnessMap    = vec2(texelFetch(materialsTexture, ivec2(0, 20  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 21 + materialID * mtl_size), 0).r);    
    mtl.hasAoMap        = int(texelFetch(materialsTexture,  ivec2(0, 22  + materialID * mtl_size), 0).r);
    mtl.aoMap           = vec2(texelFetch(materialsTexture, ivec2(0, 23  + materialID * mtl_size), 0).r, texelFetch(materialsTexture, ivec2(0, 24 + materialID * mtl_size), 0).r);    

    // Set the fragment position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_position, 1.0);
}