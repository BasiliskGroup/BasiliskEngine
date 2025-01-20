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

struct LightResult {
    vec3 diffuse;
    vec3 specular;
    vec3 clearcoat;
};

in vec2 uv;
in vec3 position;
in mat3 TBN;

// Material attributes
flat in Material mtl;

// Uniforms
uniform vec3 cameraPosition;
const int    maxDirLights = 5;
uniform      DirectionalLight dirLights[maxDirLights];
uniform int  numDirLights;
uniform      textArray textureArrays[5];
uniform samplerCube skyboxTexture;


float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

float sqr(float x) { 
    return x * x; 
}

float SchlickFresnel(float x) {
    x = clamp(1.0 - x, 0.0, 1.0);
    float x2 = x * x;
    return x2 * x2 * x;
}

float GTR1(float ndoth, float a) {
    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * ndoth * ndoth;
    return (a2 - 1.0) / (3.1415 * log(a2) * t);
}

float AnisotropicGTR2(float ndoth, float hdotx, float hdoty, float ax, float ay) {
    return 1 / (3.1415 * ax * ay * sqr(sqr(hdotx / ax) + sqr(hdoty / ay) + sqr(ndoth)));
}

float SmithGGX(float alpha, float ndotl, float ndotv) {
    float a = ndotv * sqrt(alpha + ndotl * (ndotl - alpha * ndotl));
    float b = ndotl * sqrt(alpha + ndotv * (ndotv - alpha * ndotv));

    return 0.5 / (a + b);
}

float AnisotropicSmithGGX(float ndots, float sdotx, float sdoty, float ax, float ay) {
    return 1 / (ndots + sqrt(pow(sdotx * ax, 2) + pow(sdoty * ay, 2) + pow(ndots, 2)));
}

// Diffuse model as outlined by Burley: https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Much help from Acerola's video on the topic: https://www.youtube.com/watch?v=KkOkx0FiHDA&t=570s
LightResult PrincipledDiffuse(DirectionalLight light, Material mtl, vec3 albedo, float roughness, vec3 N, vec3 V, vec3 X, vec3 Y) {

    LightResult result;

    vec3 L = normalize(-light.direction);        // light direction
    vec3 H = normalize(L + V);                      // half vector

    // Commonly used values
    float cos_theta_l = clamp(dot(N, L), 0.0, 1.0);
    float cos_theta_V = clamp(dot(N, V), 0.0, 1.0);
    float cos_theta_D = clamp(dot(L, H), 0.0, 1.0); // Also equal to dot(V, H) by symetry

    float ndoth = dot(N, H);
    float hdotx = dot(H, X);
    float hdoty = dot(H, Y);
    float ldotx = dot(L, X);
    float ldoty = dot(L, Y);
    float vdotx = dot(V, X);
    float vdoty = dot(V, Y);

    // Color Values
    vec3 surfaceColor = albedo;
    float Cdlum = luminance(surfaceColor);

    vec3 Ctint = Cdlum > 0.0 ? surfaceColor / Cdlum : vec3(1.0, 1.0, 1.0);
    vec3 Cspec0 = mix(mtl.specular * 0.08 * mix(vec3(1.0, 1.0, 1.0), Ctint, mtl.specularTint), surfaceColor, mtl.metallicness);
    vec3 Csheen = mix(vec3(1.0, 1.0, 1.0), Ctint, mtl.sheenTint);

    // Diffuse
    float FL = SchlickFresnel(cos_theta_l);
    float FV = SchlickFresnel(cos_theta_V);
    float Fss90 = cos_theta_D * cos_theta_D * roughness;
    float Fd90 = 0.5 + 2.0 * Fss90;

    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Subsurface
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * ((1 / (cos_theta_l + cos_theta_V)) - 0.5) + 0.5);

    // Specular
    float alpha = roughness * roughness;
    float aspect = sqrt(1.0 - 0.9 * mtl.anisotropic);
    float alpha_x = max(0.001, alpha / aspect);
    float alpha_y = max(0.001, alpha * aspect);


    // Anisotropic Microfacet Normal Distribution
    float Ds = AnisotropicGTR2(ndoth, hdotx, hdoty, alpha_x, alpha_y);

    // Geometric Attenuation
    float GalphaSquared = pow(0.5 + roughness * 0.5, 2);
    float GalphaX = max(0.001, GalphaSquared / aspect);
    float GalphaY = max(0.001, GalphaSquared * aspect);
    float G = AnisotropicSmithGGX(cos_theta_l, ldotx, ldoty, GalphaX, GalphaY);
    G = sqrt(G);
    G *= AnisotropicSmithGGX(cos_theta_V, vdotx, vdoty, GalphaX, GalphaY);

    // Fresnel Reflectance
    float FH = SchlickFresnel(cos_theta_D);
    vec3 F = mix(Cspec0, vec3(1.0, 1.0, 1.0), FH);

    // Sheen lobe
    vec3 Fsheen = FH * mtl.sheen * Csheen;

    // Clearcoat
    float Dr = GTR1(ndoth, mix(0.1, 0.001, mtl.clearcoatGloss)); // Normalized Isotropic GTR Gamma == 1
    float Fr = mix(0.04, 1.0, FH);
    float Gr = SmithGGX(cos_theta_l, cos_theta_V, 0.25);

    // Result for all lobes

    result.diffuse =   max(vec3(0.0), (1 / 3.1415) * albedo * (mix(Fd, ss, mtl.subsurface) + Fsheen) * (1.0 - mtl.metallicness) * cos_theta_l);
    result.specular =  max(vec3(0.0), vec3(Ds * F * G) * cos_theta_l);
    result.clearcoat = max(vec3(0.0), vec3(0.25 * mtl.clearcoat * Gr * Fr * Dr) * cos_theta_l);

    // Combine lobes and multiply weakening factor and light attributes
    return result;
}

vec3 getAlbedo(Material mtl, vec2 uv, float gamma) {
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

float getAo(Material mtl, vec2 uv) {
    float ao;
    if (bool(mtl.hasAoMap)){
        ao = texture(textureArrays[int(round(mtl.aoMap.x))].array, vec3(uv, round(mtl.aoMap.y))).a;
    }
    else {
        ao = 1.0;
    }
    return ao;
}

float getRoughness(Material mtl, vec2 uv) {
    float roughness;
    if (bool(mtl.hasRoughnessMap)){
        roughness = texture(textureArrays[int(round(mtl.roughnessMap.x))].array, vec3(uv, round(mtl.roughnessMap.y))).a;
    }
    else {
        roughness = mtl.roughness;
    }
    return roughness;
}

void main() {
    float gamma = 2.2;
    vec3 viewDir = vec3(normalize(cameraPosition - position));

    // Get lighting vectors
    vec3 albedo     = getAlbedo(mtl, uv, gamma);
    vec3 normal     = getNormal(mtl, TBN);
    float ao        = getAo(mtl, uv);
    float roughness = getRoughness(mtl, uv);
    vec3 tangent    = TBN[0];
    vec3 bitangent  = TBN[1];

    // Orthogonalize the tangent and bitangent according to the mapped normal vector
    tangent = tangent - dot(normal, tangent) * normal;
    bitangent = bitangent - dot(normal, bitangent) * normal - dot(tangent, bitangent) * tangent;

    // Lighting variables
    vec3 N = normalize(normal);                     // normal
    vec3 V = normalize(cameraPosition - position);  // view vector
    vec3 X = normalize(tangent);                    // Tangent Vector
    vec3 Y = normalize(bitangent);                  // Bitangent Vector

    // Indirect lighting
    vec3 ambient_sky = texture(skyboxTexture, N).rgb;
    vec3 reflect_sky = texture(skyboxTexture, reflect(-V, N)).rgb;

    LightResult lightResult;
    lightResult.diffuse   = vec3(0.0);
    lightResult.specular  = vec3(0.0);
    lightResult.clearcoat = vec3(0.0);

    // Add result from each directional light in the scene
    for (int i = 0; i < numDirLights; i++) {
        // Caculate the light for the directional light
        LightResult dirLightResult = PrincipledDiffuse(dirLights[i], mtl, albedo, roughness, N, V, X, Y);
        vec3 lightFactor = dirLights[i].intensity * dirLights[i].color;
        // Add each lobe
        lightResult.diffuse   += dirLightResult.diffuse   * lightFactor;
        lightResult.specular  += dirLightResult.specular  * lightFactor;
        lightResult.clearcoat += dirLightResult.clearcoat * lightFactor;
    }

    lightResult.specular =  min(vec3(1.0), lightResult.specular);
    lightResult.specular *= mix(vec3(1.0), reflect_sky, mtl.metallicness) * luminance(reflect_sky);
    lightResult.diffuse  *= mix(vec3(1.0), ambient_sky, 0.25);
    lightResult.diffuse  *= ao;

    vec3 finalColor = albedo * 0.3 * mix(vec3(1.0), reflect_sky, mtl.metallicness);
    finalColor += (lightResult.diffuse + lightResult.specular + lightResult.clearcoat);

    // light_result *= mix(vec3(1.0), texture(skyboxTexture, reflect(-V, N)).rgb, mtl.metallicness);
    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0/gamma));

    // Output fragment color
    fragColor = vec4(finalColor, 1.0);
}