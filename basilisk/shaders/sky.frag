#version 330 core

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 bloomColor;

in vec3 texCubeCoords;

uniform samplerCube skyboxTexture;

void main() {
    const float gamma = 2.2;
    const float exposure = 1.0;
    vec3 ldrColor = texture(skyboxTexture, texCubeCoords).rgb;

    // Inverse gamma correction (sRGB to linear)
    ldrColor = pow(ldrColor, vec3(gamma));

    // Apply exposure (pseudo-HDR)
    vec3 hdrColor = ldrColor * exposure;

    fragColor = vec4(hdrColor, 1.0);

    bloomColor = vec4(0.0, 0.0, 0.0, 1.0);
}