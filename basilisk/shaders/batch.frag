#version 330 core

layout (location = 0) out vec4 fragColor;

// Structs needed for the shader
struct textArray {
    sampler2DArray array;
};

struct DirLight {
    vec3 direction;
  
    vec3 color;

    float ambient;
    float diffuse;
    float specular;
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
flat in vec3 mtlColor;
flat in float roughness;
flat in float metallicness;
flat in float specular;
flat in int hasAlbedoMap;
flat in vec2 albedoMap;
flat in int hasNormalMap;
flat in vec2 normalMap;

// Uniforms
uniform vec3 cameraPosition;


vec3 CalcDirLight(DirLight light, Material mtl, vec3 normal, vec3 viewDir, vec3 albedo) {
    // Vector between the view and light vectors
    vec3 halfVector = normalize((viewDir - light.direction) / 2);
    // Lambertian Diffuse
    float diff = max(dot(normalize(-light.direction), normal) / 2 + 0.5, 0.0);
    // Blinn-Phong Specular
    float specular = pow(max(dot(normal, halfVector), 0.0), mtl.roughness);

    // Final result
    return (diff * light.diffuse + specular * light.specular) * albedo;
}


uniform textArray textureArrays[5];


void main() {
    float gamma = 2.2;
    Material mtl = Material(mtlColor, roughness, metallicness, specular, hasAlbedoMap, albedoMap, hasNormalMap, normalMap);
    DirLight dirLight = DirLight(normalize(vec3(1.5, -2.0, 1.0)), vec3(1.0, 1.0, 1.0), 0.05, 0.8, 1.0);

    vec3 albedo = mtl.color;
    if (bool(mtl.hasAlbedoMap)) {
        albedo = pow(texture(textureArrays[int(round(mtl.albedoMap.x))].array, vec3(uv, round(mtl.albedoMap.y))).rgb, vec3(gamma)) * mtl.color;
    }

    vec3 normalDirection = normal;
    if (bool(mtl.hasNormalMap)) {
        vec3 nomral_map_fragment = texture(textureArrays[int(round(mtl.normalMap.x))].array, vec3(uv, round(mtl.normalMap.y))).rgb;
        normalDirection = nomral_map_fragment * 2.0 - 1.0;
        normalDirection = normalize(TBN * normalDirection); 
    }

    vec3 viewDir = vec3(normalize(cameraPosition - position));

    vec3 light_result = CalcDirLight(dirLight, mtl, normalDirection, viewDir, albedo);

    fragColor = vec4(light_result, 1.0);
    // Gamma correction
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
}