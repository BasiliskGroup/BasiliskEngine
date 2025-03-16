#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;



void main()
{ 
    const float gamma = 2.2;
    const float exposure = 1.2;
    vec3 hdrColor = texture(screenTexture, uv).rgb + texture(bloomTexture, uv).rgb / 2;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    fragColor = vec4(mapped, 1.0);
    //fragColor = texture(screenTexture, uv) + texture(bloomTexture, uv) / 20000.0;
}