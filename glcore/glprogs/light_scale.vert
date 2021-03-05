#version 330 core

layout (location = 0) in vec2 aPos;

uniform vec4 color;
uniform mat4 proj;
uniform mat4 modelView;

out vec4 ResultColor;

void main() {
	gl_Position = proj * modelView * vec4(aPos, 0, 1);
	ResultColor = color;
}
