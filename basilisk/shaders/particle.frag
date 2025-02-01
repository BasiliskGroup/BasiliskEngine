#version 330 core
out vec4 fragColor;

in vec2 uv;
in vec3 position;
in mat3 TBN;

struct textArray {
    sampler2DArray array;
};
uniform textArray textureArrays[5];

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
flat in Material mtl;


vec3 getColor(Material mtl, vec2 uv, float gamma) {
    vec3 albedo = mtl.color;
    if (bool(mtl.hasAlbedoMap)){
        albedo *= pow(texture(textureArrays[int(round(mtl.albedoMap.x))].array, vec3(uv, round(mtl.albedoMap.y))).rgb, vec3(gamma));
    }
    return albedo;
}

vec3 getNormal(Material mtl, mat3 TBN){
    // Isolate the normal vector from the TBN basis
    vec3 normal = TBN[2];
    // Apply normal map if the material has one
    if (bool(mtl.hasNormalMap)) {
        normal = texture(textureArrays[int(round(mtl.normalMap.x))].array, vec3(uv, round(mtl.normalMap.y))).rgb * 2.0 - 1.0;
        normal = normalize(TBN * normal); 
    }
    // Return vector
    return normal;
}

void main()
{   
    // Texture and normal data
    float gamma = 2.2;
    vec3 color  = getColor(mtl, uv, gamma);
    vec3 normal = getNormal(mtl, TBN);

    // Simple light calculations
    vec3 light = normalize(vec3(1.5, 2, 1));
    float diff = abs(dot(normal, light));

    // Get color and gamma correction
    fragColor = vec4(color * (.2 + diff), 1.0);
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
}