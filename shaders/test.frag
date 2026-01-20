#version 330 core

struct Color {
    vec4 value;
};

layout (std140) uniform testBlock {
    Color colors[2];
};

in vec2 uv;
out vec4 fragColor;

void main() {
    fragColor = vec4(vec3(1.0, uv), 1.0) * colors[1].value;
}