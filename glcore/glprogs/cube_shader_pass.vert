#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 8) in vec3 aTexCoords;

uniform mat4 modelMatrix;
uniform mat4 modelView;
uniform mat4 proj;

out vec3 TexCoords;
out vec4 VertexColor;

uniform bool HAS_TEXTURE_MATRIX;
uniform mat4 texture_matrix;

void main() {
	gl_Position = proj * modelView * vec4(aPos, 1.0);
	if (HAS_TEXTURE_MATRIX) {
		TexCoords = vec3(texture_matrix * vec4(aTexCoords, 1.0));
	} else {
		TexCoords = aTexCoords;
	}
	VertexColor = aColor;
}
