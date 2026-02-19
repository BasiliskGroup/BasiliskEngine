#version 330

uniform samplerBuffer materials;

struct Material {
    vec3 color;

    uint albedoArray;
    uint albedoIndex;
    uint normalArray;
    uint normalIndex;

    float roughness;
    float subsurface;
    float sheen;
    float sheenTint;
    float anisotropic;
    float specular;
    float metallicness;
    float clearcoat;
    float clearcoatGloss;
};

Material getMaterial(int index) {
    Material m;

    int N = 4;  // N = (number of floats per material / 4)
    int base = index * N;

    vec4 v0 = texelFetch(materials, base + 0);
    vec4 v1 = texelFetch(materials, base + 1);
    vec4 v2 = texelFetch(materials, base + 2);
    vec4 v3 = texelFetch(materials, base + 3);

    // Unpack to struct fields:
    m.color         = v0.rgb;
    m.albedoArray   = floatBitsToUint(v0.a);
    m.albedoIndex   = floatBitsToUint(v1.x);
    m.normalArray   = floatBitsToUint(v1.y);
    m.normalIndex   = floatBitsToUint(v1.z);
    m.roughness     = v1.w;
    m.subsurface    = v2.x;
    m.sheen         = v2.y;
    m.sheenTint     = v2.z;
    m.anisotropic   = v2.w;
    m.specular      = v3.x;
    m.metallicness  = v3.y;
    m.clearcoat     = v3.z;
    m.clearcoatGloss= v3.w;

    return m;
}