#define MAX_DIRECTIONAL_LIGHTS 5
#define MAX_POINT_LIGHTS 50

uniform int uDirectionalLightCount;
uniform int uPointLightCount;

struct DirectionalLight {
    vec3 color;
    float intensity;
    vec3 direction;
};

struct PointLight {
    vec3 color;
    float intensity;
    vec3 position;
    float range;
};

struct AmbientLight {
    vec3 color;
};

layout (std140) uniform lights {
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    AmbientLight ambientLight;
};