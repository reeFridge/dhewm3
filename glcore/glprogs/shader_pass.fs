#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture_0;
uniform vec4 color;

void main() {
	vec4 texture_color = texture(texture_0, TexCoords);
	FragColor = color * texture_color;
}
