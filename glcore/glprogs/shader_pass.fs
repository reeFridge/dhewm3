#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture_0;

void main() {
	vec4 texture_color = texture(texture_0, TexCoords);
	FragColor = texture_color;
}
