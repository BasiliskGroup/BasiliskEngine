#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in float in_id;

uniform samplerBuffer uMatrices;

uniform mat4 view;
uniform mat4 projection;

out float id;
out vec2 uv;
out vec3 normal;


mat4 getModelMatrix(int id) {
    mat4 matrix = mat4(
        texelFetch(uMatrices, id * 4),
        texelFetch(uMatrices, id * 4 + 1),
        texelFetch(uMatrices, id * 4 + 2),
        texelFetch(uMatrices, id * 4 + 3)
    );

    return matrix;
}


void main() {
    id = in_id;
    normal = in_normal;
    uv = in_uv;

    mat4 model = getModelMatrix(int(id));
    gl_Position = projection * view * model * vec4(in_position, 1.0);
}
