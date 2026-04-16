#version 330 core

in vec2 uv;

uniform usampler2D uTexture;

out vec4 fragColor;

void main(){
    uint cell = texture(uTexture, uv).r;
    uint mat = (cell >> 24u) & 0x1Fu;
    uint fire = (cell >> 29u) & 1u;
    if (mat == 0u) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
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
