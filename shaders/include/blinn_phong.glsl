#include "light.glsl"

float getAttenuation(float distance, float range) {
    float q = 1.0 / (range * range);
    float attenuation = 1.0 / (1.0 + q * distance * distance);

    float fade = 1.0 - smoothstep(0.8 * range, range, distance);
    // float fade = clamp((range - distance) / (0.2 * range), 0.0, 1.0);
    return attenuation * fade;
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 N, vec3 V) {
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(L + V);

    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64);

    return light.color * light.intensity * (diffuse + specular);
}

vec3 calculatePointLight(PointLight light, vec3 fragPosition, vec3 N, vec3 V) {
    vec3 toLight = light.position - fragPosition;
    
    float distSquared = dot(toLight, toLight);
    float rangeSquared = light.range * light.range;
    if (distSquared > rangeSquared) return vec3(0.0);

    float distance = sqrt(distSquared);
    vec3 L = toLight * inversesqrt(distSquared);
    vec3 H = normalize(L + V);

    
    float ndotl = dot(N, L);
    if (ndotl <= 0.0) return vec3(0.0);
    float diffuse = max(ndotl, 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64);
    float attenuation = getAttenuation(distance, light.range);

    return light.color * light.intensity * attenuation * (diffuse + specular);
}