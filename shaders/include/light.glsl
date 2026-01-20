#define MAX_DIRECTIONAL_LIGHTS 5
#define TILE_SIZE 32

uniform int uDirectionalLightCount;
uniform int uPointLightCount;
uniform usamplerBuffer uLightTiles;
uniform usamplerBuffer uLightIndices;
uniform samplerBuffer uPointLights;
uniform vec3 uAmbientLight;

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

struct Tile {
    uint offset;
    uint count;
};

layout (std140) uniform uDirectionalLights {
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
};

PointLight getPointLight(int index) {
    vec4 t1 = texelFetch(uPointLights, 2 * index);
    vec4 t2 = texelFetch(uPointLights, 2 * index + 1);

    PointLight light;
    light.color = t1.rgb;
    light.intensity = t1.a;
    light.position = t2.xyz;
    light.range = t2.w;

    return light;
}

Tile getTile() {
    int tilesX = int(ceil(800.0 / float(TILE_SIZE)));

    ivec2 tileCoord = ivec2(gl_FragCoord.xy) / TILE_SIZE;
    int tileIndex = tileCoord.y * tilesX + tileCoord.x;

    uvec4 texelData = texelFetch(uLightTiles, tileIndex);

    Tile tile;
    tile.offset = texelData.x;
    tile.count  = texelData.y;
    return tile;
}

PointLight getLightByIndex(uint linearIndex) {
    int texelIndex = int(linearIndex) / 4;
    int component  = int(linearIndex) % 4;

    uvec4 texelData = texelFetch(uLightIndices, texelIndex);
    uint lightIndex = texelData[component];

    return getPointLight(int(lightIndex));
}