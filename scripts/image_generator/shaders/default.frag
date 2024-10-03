#version 330 core

layout (location = 0) out vec4 fragColor;

in vec4 vert;
in vec3 norm;

void main() {
    float shading = ((dot(normalize(vec3(-.25, .5, .75)), norm) + 1) / 2) * .75 + .25;
    fragColor = vec4(vec3(shading), 1.0);
}