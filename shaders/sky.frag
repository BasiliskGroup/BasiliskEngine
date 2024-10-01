#version 330 core

layout (location = 0) out vec4 frag_color;

uniform vec4 fogColor;
uniform vec4 planeColor;

in vec3 v_viewpos;

float NEAR = 0.1;
float FAR = 150.0;

void main() {
    float fog = smoothstep(NEAR, FAR, length(v_viewpos));
    frag_color = vec4(mix(planeColor, fogColor, fog).rgb, 1.0);
}