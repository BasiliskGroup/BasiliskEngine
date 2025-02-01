#version 330 core
out vec4 fragColor;

in vec3 color;
in vec3 normal;

void main()
{             
    vec3 light = normalize(vec3(1.5, 2, 1));
    float diff = abs(dot(normal, light));
    fragColor = vec4(color * (.2 + diff), 1.0);
}