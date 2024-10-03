#version 330

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

uniform mat4 m_proj;
uniform mat4 m_view;
uniform mat4 m_model;

out vec4 vert;
out vec3 norm;

void main() {
	vert = m_proj * m_view * m_model * vec4(in_position, 1.0);
	norm = in_normal;

	gl_Position = m_proj * m_view * m_model * vec4(in_position, 1.0);
}