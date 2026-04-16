#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
uniform vec2 uBufferSize;
uniform float uPointSize;
out vec3 vColor;
void main() {
    vec2 ndc;
    ndc.x = (aPos.x / uBufferSize.x) * 2.0 - 1.0;
    ndc.y = (aPos.y / uBufferSize.y) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    gl_PointSize = uPointSize;
    vColor = aColor;
}
