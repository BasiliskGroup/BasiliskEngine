#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;


in vec3  in_instance_pos;
in vec3  in_instance_color;
in float scale;
in float life;

out vec3 color;
out vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main() {
    // Pass data forward
    color  = in_instance_color;
    normal = in_normal;

    // Get the model matrix
    vec3  instance_pos = in_instance_pos;
    float size = scale * life;
    mat4  modelMatrix = mat4(
        size, 0.0, 0.0, 0.0,
        0.0, size, 0.0, 0.0,
        0.0, 0.0, size, 0.0,
        instance_pos.x, instance_pos.y, instance_pos.z, 1.0
    );

    // Send position to the frag
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_position, 1.0);
}