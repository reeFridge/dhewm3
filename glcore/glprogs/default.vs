#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 8) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 modelView;
uniform mat4 proj;

out vec2 f_texCoords;

void main()
{
	vec4 vertex_position = proj * modelView * vec4(aPos, 1.0);
	gl_Position = vertex_position;
	f_texCoords = aTexCoords;
}
