#version 330 core
out vec4 fragColor;

in vec3 texCubeCoords;

uniform samplerCube skyboxTexture;

void main() {
    fragColor = texture(skyboxTexture, texCubeCoords);
}