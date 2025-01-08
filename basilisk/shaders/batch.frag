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

float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

float sqr(float x) { 
    return x * x; 
}

float SchlickFresnel(float x) {
    x = clamp(1.0 - x, 0.0, 1.0);
    float x2 = x * x;

    return x2 * x2 * x; // While this is equivalent to pow(1 - x, 5) it is two less mult instructions
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
vec3 PrincipledDiffuse(DirectionalLight light, Material mtl, vec3 albedo, vec3 N, vec3 L, vec3 V, vec3 H, vec3 X, vec3 Y) {

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
    float Fss90 = cos_theta_D * cos_theta_D * mtl.roughness;
    float Fd90 = 0.5 + 2.0 * Fss90;

    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Subsurface
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * ((1 / (cos_theta_l + cos_theta_V)) - 0.5) + 0.5);

    // Specular
    float alpha = mtl.roughness * mtl.roughness;
    float aspect = sqrt(1.0 - 0.9 * mtl.anisotropic);
    float alpha_x = max(0.001, alpha / aspect);
    float alpha_y = max(0.001, alpha * aspect);


    // Anisotropic Microfacet Normal Distribution
    float Ds = AnisotropicGTR2(ndoth, hdotx, hdoty, alpha_x, alpha_y);

    // Geometric Attenuation
    float GalphaSquared = pow(0.5 + mtl.roughness * 0.5, 2);
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

    // Lobes
    vec3 specular = vec3(Ds * F * G);
    vec3 diffuse  = (1 / 3.1415) * albedo * (mix(Fd, ss, mtl.subsurface) + Fsheen) * (1.0 - mtl.metallicness);
    vec3 clearcoat = vec3(0.25 * mtl.clearcoat * Gr * Fr * Dr);


    return (diffuse + specular + clearcoat) * cos_theta_l * light.intensity * light.color;
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
    vec3 tangent   = TBN[0];
    vec3 bitangent = TBN[1];

    tangent = tangent - dot(normal, tangent) * normal;
    bitangent = bitangent - dot(normal, bitangent) * normal - dot(tangent, bitangent) * tangent;

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