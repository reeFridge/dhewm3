#version 330 core

layout (location = 0) in vec4 aPos;

uniform vec4 light_origin;

uniform mat4 model;
uniform mat4 modelView;
uniform mat4 proj;

void main() {
	mat4 mvp = proj * modelView;
	vec4 r0 = aPos - light_origin;
	r0 = vec4(r0.w) * light_origin + r0;

	gl_Position = mvp * r0;
}
