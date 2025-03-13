#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D screenTexture;

float warp = 0.75; // simulate curvature of CRT monitor
float scan = 0.75; // simulate darkness between scanlines

void main(){
    // squared distance from center
    vec2 crt_uv = uv;
    vec2 dc = abs(0.5-uv);
    dc *= dc;
    
    // warp the fragment coordinates
    crt_uv.x -= 0.5; crt_uv.x *= 1.0+(dc.y*(0.3*warp)); crt_uv.x += 0.5;
    crt_uv.y -= 0.5; crt_uv.y *= 1.0+(dc.x*(0.4*warp)); crt_uv.y += 0.5;

    // sample inside boundaries, otherwise set to black
    if (crt_uv.y > 1.0 || crt_uv.x < 0.0 || crt_uv.x > 1.0 || crt_uv.y < 0.0) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else {
        // determine if we are drawing in a scanline
        float apply = abs(sin(crt_uv.y * 300)*0.5*scan);
        //sample the texture
    	fragColor = vec4(mix(texture(screenTexture, crt_uv).rgb,vec3(0.0), apply), 1.0);
    }
}