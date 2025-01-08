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
    vec3  color;
    float roughness;
    float subsurface;
    float sheen;
    vec3  sheenTint;
    float anisotropic;
    float specular;
    float metallicness;
    vec3  specularTint;
    float clearcoat;
    float clearcoatGloss;
    
    int   hasAlbedoMap;
    vec2  albedoMap;
    int   hasNormalMap;
    vec2  normalMap;
};

in vec2 uv;
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

// Diffuse model as outlined by Burley: https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Much help from Acerola's video on the topic: https://www.youtube.com/watch?v=KkOkx0FiHDA&t=570s
vec3 PrincipledDiffuse(DirectionalLight light, Material mtl, vec3 albedo, vec3 N, vec3 L, vec3 V, vec3 H, vec3 X, vec3 Y) {

    // Diffuse from roughness of mtl
    float cos_theta_l = max(dot(N, L), 0.0);
    float cos_theta_V = max(dot(N, V), 0.0);
    float cos_theta_D = max(dot(L, H), 0.0); // Also equal to dot(V, H) by symetry

    float FL = SchlickFresnel(cos_theta_l);
    float FV = SchlickFresnel(cos_theta_V);

    float Fss90 = cos_theta_D * cos_theta_D * mtl.roughness;
    float Fd90 = 0.5 + 2.0 * Fss90;

    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Sheen lobe
    vec3 fS = mtl.sheen * pow((1 - cos_theta_D), 5) * mtl.sheenTint;

    // Subsurface
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * ((1 / (cos_theta_l + cos_theta_V)) - 0.5) + 0.5);

    // Specular
    // float alpha = mtl.roughness * mtl.roughness;
    // float aspect = sqrt(1 - 0.9 * mtl.anisotropic);
    // float alpha_x = alpha / aspect;
    // float alpha_y = alpha * aspect;

    // vec3 D_GTR_aniso = (1 / 3.1415) * (alpha_x / alpha_y) * (1 / ((H )))

    return ((1 / 3.1415) * albedo * mix(Fd, ss, mtl.subsurface) + fS) * cos_theta_l * light.intensity * light.color;
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

    vec3 albedo    = getAlbedo(mtl, uv, gamma);
    vec3 normal    = getNormal(mtl, TBN[2], TBN);
    vec3 tangent   = getNormal(mtl, TBN[0], TBN);
    vec3 bitangent = getNormal(mtl, TBN[1], TBN);

    // Lighting variables
    vec3 N = normalize(normal);                     // normal
    vec3 L = normalize(-dirLight.direction);        // light direction
    vec3 V = normalize(cameraPosition - position);  // view vector
    vec3 H = normalize(L + V);                      // half vector
    vec3 X = normalize(tangent);                    // Tangent Vector
    vec3 Y = normalize(bitangent);                  // Bitangent Vector

    vec3 light_result = PrincipledDiffuse(dirLight, mtl, albedo, N, L, V, H, X, Y) + dirLight.ambient;
    light_result = pow(light_result, vec3(1.0/gamma));

    fragColor = vec4(light_result, 1.0);
}