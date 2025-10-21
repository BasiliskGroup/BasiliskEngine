#version 330 core

in float id;
in vec3 normal;
in vec2 uv;

out vec4 fragColor;

uniform sampler2D texture1;


void main() {

    vec3 globalLight = normalize(vec3(.5, 1, .25));
    float brightness = (dot(normal, globalLight) + 1) / 2;

    fragColor = brightness * texture(texture1, uv);
} 