#version 330 core

in vec3 uv;
uniform samplerCube skybox;

out vec4 fragColor;

void main() {    
    fragColor = texture(skybox, uv);
}