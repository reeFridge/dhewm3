#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 9) in vec3 aNormal;

uniform vec4 local_eye_pos;

uniform mat4 modelView;
uniform mat4 proj;

out vec4 VertexColor;
out vec3 Normal;
out vec3 ToEye;

void main() {
	Normal = aNormal;
	ToEye = local_eye_pos.xyz - aPos;
	gl_Position = proj * modelView * vec4(aPos, 1.0);
	VertexColor = aColor;
}
