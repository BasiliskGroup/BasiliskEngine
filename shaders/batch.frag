#version 330 core

layout (location = 0) out vec4 fragColor;


in vec2 uv;
flat in int materialID;
in vec3 normal;
in vec3 position;


uniform vec3 cameraPosition;


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
    int hasSpecularMap;
    int hasNormalMap;

    vec2 albedoMap;
    vec2 specularMap;
    vec2 normalMap;
};

uniform DirLight dirLight;
#define maxLights 100
uniform int numPointLights;
uniform PointLight pointLights[maxLights];

#define maxMaterials 10
uniform Material materials[maxMaterials];

vec3 CalcDirLight(DirLight light, Material mtl, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse
    float diff = max((dot(normal, lightDir) + 1) / 2, 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mtl.specularExponent);
    // result
    vec3 ambient  = light.ambient  * albedo * light.color * mtl.color;
    vec3 diffuse  = light.diffuse  * diff * albedo * light.color * mtl.color;
    vec3 specular = light.specular * spec * albedo * light.color * mtl.specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, Material mtl, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
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


    Material mtl = materials[int(materialID)];

    vec3 albedo;
    vec2 textureID;
    if (bool(mtl.hasAlbedoMap)) {
        textureID = mtl.albedoMap;
        albedo = texture(textureArrays[int(round(textureID.x))].array, vec3(uv, round(textureID.y))).rgb;
    }
    else {
        albedo = mtl.color;
    }

    vec3 normalDirection = normal;
    if (bool(mtl.hasNormalMap)) {
        textureID = mtl.normalMap;
        normalDirection = normalize(normal + texture(textureArrays[int(round(textureID.x))].array, vec3(uv, round(textureID.y))).rgb);
    }

    vec3 viewDir = vec3(normalize(cameraPosition - position));

    vec3 light_result = CalcDirLight(dirLight, mtl, normalDirection, viewDir, albedo);
    for(int i = 0; i < numPointLights; i++){
        float distance = length(pointLights[i].position - position);
        light_result += CalcPointLight(pointLights[i], mtl, normalDirection, position, viewDir, albedo);
        //if (distance < pointLights[i].radius){
        //    light_result += CalcPointLight(pointLights[i], mtl, normalDirection, position, viewDir, albedo);
        //}
    }

    fragColor = vec4(light_result, mtl.alpha);
    //fragColor.rgb *= 0.0001;
    //fragColor.rgb += viewDir;
}