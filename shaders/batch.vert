#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

layout (location = 3) in vec3 obj_position;
layout (location = 4) in vec3 obj_rotation;
layout (location = 5) in vec3 obj_scale;
layout (location = 6) in int  obj_material;

out vec2 uv;
flat out int  materialID;
out vec3 normal;
out vec3 position;

uniform mat4 m_proj;
uniform mat4 m_view;

void main() {
    vec3 rot = obj_rotation;

    mat4 m_rot = mat4(
        cos(rot.z) * cos(rot.y), cos(rot.z) * sin(rot.y) * sin(rot.x) - sin(rot.z) * cos(rot.x), cos(rot.z) * sin(rot.y) * cos(rot.x) + sin(rot.z) * sin(rot.x), 0,
        sin(rot.z) * cos(rot.y), sin(rot.z) * sin(rot.y) * sin(rot.x) + cos(rot.z) * cos(rot.x), sin(rot.z) * sin(rot.y) * cos(rot.x) - cos(rot.z) * sin(rot.x), 0,
        -sin(rot.y)            , cos(rot.y) * sin(rot.x)                                       , cos(rot.y) * cos(rot.x)                                       , 0,
        0                      , 0                                                             , 0                                                             , 1
    );

    mat4 m_trans = mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        obj_position.x, obj_position.y, obj_position.z, 1
    );

    mat4 m_scale = mat4(
        obj_scale.x, 0          , 0          , 0,
        0          , obj_scale.y, 0          , 0,
        0          , 0          , obj_scale.z, 0,
        0          , 0          , 0          , 1
    );

    mat4 m_model = m_trans * m_rot * m_scale;

    position = (m_model * vec4(in_position, 1.0)).xyz;
    normal = normalize(mat3(transpose(inverse(m_model))) * in_normal);
    uv = in_uv;
    materialID = obj_material;

    gl_Position = m_proj * m_view * m_model * vec4(in_position, 1.0);
}