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

float SchlickFresnel(float value){
    return pow(clamp(1 - value, 0.0, 1.0), 5);
}

vec3 CalcDirLight(DirectionalLight light, Material mtl, vec3 N, vec3 L, vec3 H) {
    float diff = max(dot(L, N), 0.0);
    float spec = pow(max(dot(N, H), 0.0), mtl.roughness);
    // Final result
    return (diff + spec * diff * mtl.specular + light.ambient) * light.intensity * light.color;
}

vec3 CalcDirLightBurley(DirectionalLight light, Material mtl, vec3 N, vec3 L, vec3 V, vec3 H, float ndotl, float ndotv, float ldoth) {

    float FL = SchlickFresnel(ndotl);
    float FV = SchlickFresnel(ndotv);

    float F90 = mtl.roughness * pow(ldoth, 2.0);
    float Fd = mix(1.0, F90, FL) * mix(1.0, F90, FV);

    return Fd * light.intensity * (light.color / 3.1415) + light.ambient;
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
        normal = texture(textureArrays[int(round(mtl.normalMap.x))].array, vec3(uv, round(mtl.normalMap.y))).rgb * 2.0 - 1.0;
        normal = normalize(TBN * normal); 
    }
    return normal;
}

void main() {
    float gamma = 2.2;
    vec3 viewDir = vec3(normalize(cameraPosition - position));

    vec3 albedo = getAlbedo(mtl, uv, gamma);
    vec3 normal = getNormal(mtl, normal, TBN);

    // Lighting variables
    vec3 N = normalize(normal);                                // normal
    vec3 L = normalize(-dirLight.direction);                   // light direction
    vec3 V = normalize(cameraPosition - position);  // view vector
    vec3 H = normalize(L + V);                      // half vector
    float ndotl = max(dot(N, L), 0.0);
    float ndotv = max(dot(N, V), 0.0);
    float ndoth = max(dot(N, H), 0.0);
    float ldoth = dot(L, H);

    // vec3 light_result = CalcDirLight(dirLight, mtl, N, L, H) / 10000000;
    vec3 light_result = albedo * CalcDirLightBurley(dirLight, mtl, N, L, V, H, ndotl, ndotv, ldoth);
    light_result = pow(light_result, vec3(1.0/gamma));
    vec3 light_result2 = albedo * CalcDirLight(dirLight, mtl, N, L, H);
    light_result2 = pow(light_result2, vec3(1.0/gamma));

    fragColor = vec4(light_result, 1.0);
}