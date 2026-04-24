#version 330 core

in vec2 uv;

uniform usampler2D uTexture;

out vec4 fragColor;

uniform vec2 location;
uniform vec2 cameraScale;
uniform vec2 bufferSize;
uniform float cellScale;

void main(){
    vec2 offset = vec2(0.5) - vec2(cameraScale / cellScale / bufferSize) / 2.0;
    vec2 position = location / cellScale / bufferSize;
    vec2 localUV = (uv / bufferSize * cameraScale) / cellScale + position + offset;

    uint cell = texture(uTexture, localUV).r;
    uint mat = (cell >> 24u) & 0x1Fu;
    uint fire = (cell >> 29u) & 1u;
    if (mat == 0u) {
        discard;
    }

    float r = float((cell >> 16u) & 0xFFu) / 255.0;
    float g = float((cell >> 8u) & 0xFFu) / 255.0;
    float b = float(cell & 0xFFu) / 255.0;
    if (fire != 0u) {
        r = min(1.0, r * 0.35 + 0.95);
        g = min(1.0, g * 0.4 + 0.25);
        b *= 0.15;
    }
    fragColor = vec4(r, g, b, 1.0);
}
