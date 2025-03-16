#version 330 core

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 bloomColor;


void main() {
    // Output fragment color
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    bloomColor = vec4(0.0);
}