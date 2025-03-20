#version 330 core

layout (location = 0) out vec4 fragColor;

in vec2 uv;
uniform sampler2D depthTexture;
uniform sampler2D normalTexture;
uniform vec2 viewportDimensions;
uniform float near;
uniform float far;

const float width = 1.0;

float linearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{ 
    vec2 offset = 1.0 / viewportDimensions * width;  

    // List of offsets used for sampling the texture
    vec2 offsets[9] = vec2[](
        vec2(-1.0, -1.0) * offset,
        vec2(-1.0,  0.0) * offset,
        vec2(-1.0,  1.0) * offset,
        vec2( 0.0, -1.0) * offset,
        vec2( 0.0,  0.0) * offset,
        vec2( 0.0,  1.0) * offset,
        vec2( 1.0, -1.0) * offset,
        vec2( 1.0,  0.0) * offset,
        vec2( 1.0,  1.0) * offset 
    );

    float kernel[9] = float[](
        1, 1, 1,
        1,-8, 1,
        1, 1, 1
    );

    float depth_total = 0.0;
    vec3 normal_total = vec3(0.0);
    for (int i = 0; i < 9; i++)
    {
       depth_total  += linearizeDepth(texture(depthTexture, clamp(uv + offsets[i], 0.001, 0.999)).r) * kernel[i];
       normal_total += texture(normalTexture, clamp(uv + offsets[i], 0.001, 0.999)).rgb * kernel[i];
    }

    float t_depth = 4;
    float t_norm = 0.05;
    fragColor = vec4(int(depth_total > t_depth || normal_total.r > t_norm || normal_total.g > t_norm || normal_total.b > t_norm));
}