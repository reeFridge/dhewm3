#version 330 core

in vec2 TexCoords;

uniform bool alpha_test;
uniform float alpha_test_value;
uniform vec4 color;

uniform sampler2D texture_0;

out vec4 FragColor;

void main()
{
	vec4 result = texture(texture_0, TexCoords) * color;
	if (alpha_test && result.a <= alpha_test_value) {
		discard;
	}

	FragColor = result;
}
