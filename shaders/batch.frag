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

struct PointLight{
    vec3 position;

    vec3 color;

    float constant;
    float linear;
    float quadratic;  

    float ambient;
    float diffuse;
    float specular;
    float radius;
};

struct Material {
    vec3 color;
    float specular;
    float specularExponent;
    float alpha;

    int hasAlbedoMap;
    //int hasSpecularMap;
    int hasNormalMap;

    vec2 albedoMap;
    //vec2 specularMap;
    vec2 normalMap;
};


in vec2 uv;
in vec3 normal;
in vec3 position;
in mat3 TBN;
// Material attributes
flat in vec3 mtlColor;
flat in vec2 albedoMap;
flat in vec2 normalMap;
flat in float mtlSpecular;
flat in float mtlSpecularExponent;
flat in float mtlAlpha;
flat in int hasAlbedoMap;
flat in int hasNormalMap;

// Uniforms
uniform vec3 cameraPosition;
#define maxLights 100
uniform DirLight dirLight;
uniform int numPointLights;
uniform PointLight pointLights[maxLights];


vec3 CalcDirLight(DirLight light, Material mtl, vec3 normal, vec3 viewDir, vec3 albedo) {
    // Vector between the view and light vectors
    vec3 halfVector = normalize((viewDir - light.direction) / 2);
    // Lambertian Diffuse
    float diff = max(dot(normalize(-light.direction), normal) / 2 + 0.5, 0.0);
    // Blinn-Phong Specular
    float specular = pow(max(dot(normal, halfVector), 0.0), mtl.specularExponent);

    // Backlight
    vec3 backDirection = -light.direction;
    vec3 halfVectorBack = normalize((viewDir - backDirection) / 2);
    float diffBack = max(dot(normalize(-backDirection), normal), 0.0);
    float specularBack = max(pow(dot(normal, halfVectorBack), mtl.specularExponent), 0.0);

    // Final result
    return light.color * light.diffuse * (albedo + (specular + specularBack / 4) * mtl.specular) * (diff + diffBack / 4) + light.ambient * albedo;
}

vec3 CalcPointLight(PointLight light, Material mtl, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse
    float diff = max((dot(normal, lightDir) + 1) / 2, 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 24);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // result
    vec3 ambient  = light.ambient  * albedo * light.color * mtl.color;
    vec3 diffuse  = light.diffuse  * diff * albedo * light.color * mtl.color;
    vec3 specular = light.specular * spec * albedo * light.color * mtl.specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 


uniform textArray textureArrays[5];


void main() {
    float gamma = 2.2;
    Material mtl = Material(mtlColor, mtlSpecular, mtlSpecularExponent, mtlAlpha, hasAlbedoMap, hasNormalMap, albedoMap, normalMap);

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
    for(int i = 0; i < numPointLights; i++){
        float distance = length(pointLights[i].position - position);
        if (distance < pointLights[i].radius * 10){
            light_result += CalcPointLight(pointLights[i], mtl, normalDirection, position, viewDir, albedo);
        }
    }

    fragColor = vec4(light_result, mtl.alpha);
    // Gamma correction
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
}