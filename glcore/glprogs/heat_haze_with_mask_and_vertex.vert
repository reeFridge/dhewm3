#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 8) in vec2 aTexCoords;
layout (location = 9) in vec3 aNormal;
layout (location = 10) in vec3 aTangent0;
layout (location = 11) in vec3 aTangent1;

uniform vec4 vertex_parm_0; // scroll
uniform vec4 vertex_parm_1; // deform magnitude (1.0 is reasonable, 2.0 is twice as wavy, 0.5 is half as wavy, etc)
//uniform vec4 vertex_parm_n;

uniform mat4 model;
uniform mat4 modelView;
uniform mat4 proj;

out vec4 Color;
out vec2 TexCoords;
out vec2 TexCoords1;
out vec2 TexCoords2;

float rcp(float x) {
	return 1.0/x;
}

void main() {
	gl_Position = proj * modelView * vec4(aPos, 1.0);

	vec4 mv_z_row = transpose(modelView)[2];
	vec4 p_x_row = transpose(proj)[0];
	vec4 p_w_row = transpose(proj)[3];

	vec4 R0 = vec4(1.0, 0, 0, 1.0);
	R0.z = dot(vec4(aPos, 1.0), mv_z_row);

	float R1 = dot(R0, p_x_row);
	float R2 = max(dot(R0, p_w_row), 1.0);
	R1 = min(rcp(R2) * R1, 0.02);

	Color = aColor;
	TexCoords = aTexCoords;
	TexCoords2 = vertex_parm_1.xy * R1;
	TexCoords1 = aTexCoords + vertex_parm_0.xy;
}
