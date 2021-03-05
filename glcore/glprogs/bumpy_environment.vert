#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 8) in vec2 aTexCoords;
layout (location = 9) in vec3 aNormal;
layout (location = 10) in vec3 aTangent0;
layout (location = 11) in vec3 aTangent1;

uniform vec4 global_eye_pos;

uniform mat4 model;
uniform mat4 modelView;
uniform mat4 proj;

out vec2 tc_normal_map;
out vec3 tc_to_eye;
out vec3 tc_normal;
out vec3 tc_tangent0;
out vec3 tc_tangent1;
out vec4 VertexColor;

void main() {
	gl_Position = proj * modelView * vec4(aPos, 1.0);
	vec3 toEye = global_eye_pos.xyz - aPos;
	tc_to_eye = (model * vec4(toEye, 1.0)).xyz;
	tc_normal = (model * vec4(aNormal, 1.0)).xyz;
	tc_tangent0 = (model * vec4(aTangent0, 1.0)).xyz;
	tc_tangent1 = (model * vec4(aTangent1, 1.0)).xyz;
	tc_normal_map = aTexCoords;
	VertexColor = aColor;
}
