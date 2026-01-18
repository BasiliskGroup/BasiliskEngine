#include "light.glsl"

float getAttenuation(float distance, float range) {
    float q = 1.0 / (range * range);
    float attenuation = 1.0 / (1.0 + q * distance * distance);

    float fade = 1.0 - smoothstep(0.8 * range, range, distance);
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
    float distance = length(light.position - fragPosition);
    if (distance > light.range) {
        return vec3(0.0, 0.0, 0.0);
    }

    vec3 L = normalize(light.position - fragPosition);
    vec3 H = normalize(L + V);

    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64);
    float attenuation = getAttenuation(distance, light.range);

    return light.color * light.intensity * attenuation * (diffuse + specular);
}