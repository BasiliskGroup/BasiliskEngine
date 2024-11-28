#version 330 core

layout (location = 0) out vec4 fragColor;

// Structs needed for the shader
struct textArray {
    sampler2DArray array;
};

struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
    float ambient;
};  

struct Material {
    vec3 color;
    float roughness;
    float metallicness;
    float specular;

    int hasAlbedoMap;
    vec2 albedoMap;
    int hasNormalMap;
    vec2 normalMap;
};

in vec2 uv;
in vec3 normal;
in vec3 position;
in mat3 TBN;

// Material attributes
flat in Material mtl;

// Uniforms
uniform vec3 cameraPosition;
uniform DirectionalLight dirLight;
uniform textArray textureArrays[5];


vec3 CalcDirLight(DirectionalLight light, Material mtl, vec3 normal, vec3 viewDir, vec3 albedo) {
    // Vector between the view and light vectors
    vec3 halfVector = normalize((viewDir - light.direction) / 2);
    // Lambertian Diffuse
    float diff = max(dot(normalize(-light.direction), normal) / 2 + 0.5, 0.0);
    // Blinn-Phong Specular
    float specular = pow(max(dot(normal, halfVector), 0.0), mtl.roughness);

    // Final result
    return (diff + specular + light.ambient) * light.intensity * albedo * light.color;
}

vec3 getAlbedo(Material mtl, vec2 uv, float gamma) {
    vec3 albedo = mtl.color;
    if (bool(mtl.hasAlbedoMap)){
        albedo *= pow(texture(textureArrays[int(round(mtl.albedoMap.x))].array, vec3(uv, round(mtl.albedoMap.y))).rgb, vec3(gamma));
    }
    return albedo;
}

vec3 getNormal(Material mtl, vec3 normal, mat3 TBN){
    if (bool(mtl.hasNormalMap)) {
        vec3 nomral_map_fragment = texture(textureArrays[int(round(mtl.normalMap.x))].array, vec3(uv, round(mtl.normalMap.y))).rgb;
        normal = nomral_map_fragment * 2.0 - 1.0;
        normal = normalize(TBN * normal); 
    }
    return normal;
}

void main() {
    float gamma = 2.2;
    vec3 viewDir = vec3(normalize(cameraPosition - position));

    vec3 albedo = getAlbedo(mtl, uv, gamma);
    vec3 normal = getNormal(mtl, normal, TBN);

    vec3 light_result = CalcDirLight(dirLight, mtl, normal, viewDir, albedo);
    light_result = pow(light_result, vec3(1.0/gamma));

    fragColor = vec4(light_result, 1.0);
}