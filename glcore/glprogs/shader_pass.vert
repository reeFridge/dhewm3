#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 8) in vec2 aTexCoords;
layout (location = 9) in vec3 aNormal;
layout (location = 10) in vec3 aTangent0;
layout (location = 11) in vec3 aTangent1;

uniform mat4 modelMatrix;
uniform mat4 modelView;
uniform mat4 proj;

out vec2 TexCoords;

void main() {
	gl_Position = proj * modelView * vec4(aPos, 1.0);
	TexCoords = aTexCoords;
}
