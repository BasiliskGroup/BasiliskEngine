#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

layout (location = 5) in vec3 obj_position;
layout (location = 6) in vec4 obj_rotation;
layout (location = 7) in vec3 obj_scale;
layout (location = 8) in float obj_material;

out vec2 uv;
out vec3 normal;
out vec3 position;
out mat3 TBN;

flat out vec3 mtlColor;
flat out float roughness;
flat out float metallicness;
flat out float specular;
flat out int hasAlbedoMap;
flat out vec2 albedoMap;
flat out int hasNormalMap;
flat out vec2 normalMap;

uniform mat4 m_proj;
uniform mat4 m_view;
uniform sampler2D materialsTexture;

void main() {
    vec4 rot = obj_rotation;

    mat4 m_rot = mat4(
        1 - 2 * (rot.z * rot.z + rot.w * rot.w), 2 * (rot.y * rot.z - rot.w * rot.x), 2 * (rot.y * rot.w + rot.z * rot.x), 0,
        2 * (rot.y * rot.z + rot.w * rot.x), 1 - 2 * (rot.y * rot.y + rot.w * rot.w), 2 * (rot.z * rot.w - rot.y * rot.x), 0,
        2 * (rot.y * rot.w - rot.z * rot.x), 2 * (rot.z * rot.w + rot.y * rot.x), 1 - 2 * (rot.y * rot.y + rot.z * rot.z), 0,
        0, 0, 0, 1
    );

    mat4 m_trans = mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        obj_position.x, obj_position.y, obj_position.z, 1
    );

    mat4 m_scale = mat4(
        obj_scale.x, 0          , 0          , 0,
        0          , obj_scale.y, 0          , 0,
        0          , 0          , obj_scale.z, 0,
        0          , 0          , 0          , 1
    );

    mat4 m_model = m_trans * m_rot * m_scale;

    position = (m_model * vec4(in_position, 1.0)).xyz;

    normal = normalize(mat3(transpose(inverse(m_model))) * in_normal);
    vec3 T = normalize(vec3(m_model * vec4(in_tangent,   0.0)));
    vec3 B = normalize(vec3(m_model * vec4(in_bitangent, 0.0)));
    vec3 N = normalize(vec3(m_model * vec4(in_normal,    0.0)));
    TBN = mat3(T, B, N);
    
    uv = in_uv;
    int materialID = int(obj_material);

    // Get the material attributes from material texture
    mtlColor  = vec3(texelFetch(materialsTexture, ivec2(0, 0  + materialID * 12), 0).r, texelFetch(materialsTexture, ivec2(0, 1  + materialID * 12), 0).r, texelFetch(materialsTexture, ivec2(0, 2  + materialID * 12), 0).r);
    
    roughness    = texelFetch(materialsTexture, ivec2(0, 3  + materialID * 12), 0).r;
    metallicness = texelFetch(materialsTexture, ivec2(0, 4  + materialID * 12), 0).r;
    specular     = texelFetch(materialsTexture, ivec2(0, 5  + materialID * 12), 0).r;
    
    hasAlbedoMap = int(texelFetch(materialsTexture, ivec2(0, 6  + materialID * 12), 0).r);
    albedoMap = vec2(texelFetch(materialsTexture, ivec2(0, 7  + materialID * 12), 0).r, texelFetch(materialsTexture, ivec2(0, 8  + materialID * 12), 0).r);
    hasNormalMap = int(texelFetch(materialsTexture, ivec2(0, 9  + materialID * 12), 0).r);
    normalMap = vec2(texelFetch(materialsTexture, ivec2(0, 10 + materialID * 12), 0).r, texelFetch(materialsTexture, ivec2(0, 11 + materialID * 12), 0).r);    

    gl_Position = m_proj * m_view * m_model * vec4(in_position, 1.0);
}