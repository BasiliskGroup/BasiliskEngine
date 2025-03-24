#version 330 core

layout (location = 0) out vec4 fragColor;

in vec2 uv;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;



void main()
{ 
    const float gamma = 2.2;
    const float exposure = 1.2;

    // Sammple from textures
    vec4 color = texture(screenTexture, uv);
    vec4 bloom = texture(bloomTexture, uv);

    vec3 hdrColor = color.rgb + bloom.rgb / 2;
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    fragColor = vec4(mapped, color.a);
}