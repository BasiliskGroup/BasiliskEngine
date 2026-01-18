#version 330 core

layout (location = 0) in vec3 vPosition;

uniform mat4 uProjection;
uniform mat4 uView;

out vec3 uv;

void main() {
    uv = vPosition;
    gl_Position = uProjection * uView * vec4(vPosition, 1.0);
    // gl_Position = pos.xyww;
}  