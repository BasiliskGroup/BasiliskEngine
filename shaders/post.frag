#version 330 core

layout (location = 0) out vec4 fragColor;


in vec2 uv;

uniform sampler2D uTexture;
uniform vec2 textureSize;

const int buckets = 4;
const int n = 4;
// const float s = 0.025;
const float s = 0.05;
mat4 bayerDither = mat4 (
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
    15, 7, 13, 5
);


vec3 palette[buckets] = vec3[buckets](
    vec3(166, 77, 121) / 255,
    vec3(106, 30, 85) / 255,
    vec3(59, 28, 50)  / 255,
    vec3(26, 26, 29) / 255
);

float grayscale(vec3 color) {
    return dot(color.rgb, vec3(0.299, 0.587, 0.114));
}

void main() {
    vec4 color = texture(uTexture, uv);

    vec2 position = uv * textureSize;
    position = mod(position, n);
    float M = bayerDither[int(position.x)][int(position.y)] / (n * n) - 0.5;

    float value = grayscale(color.rgb + s * M);

    fragColor = vec4(palette[clamp(buckets - int(floor((value) * buckets)) - 1, 0, buckets - 1)], 1.0);

    // float quantized = floor((value) * buckets) / buckets;
    // fragColor = vec4(vec3(quantized), 1.0);

}