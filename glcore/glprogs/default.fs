#version 330 core

in vec2 f_texCoords;

uniform bool alpha_test;
uniform float alpha_test_value;
uniform vec4 color;
uniform sampler2D t_global0;

out vec4 FragColor;

void main()
{
	vec4 result = texture(t_global0, f_texCoords) * color;
	if (alpha_test && result.a <= alpha_test_value) {
		discard;
	}

	FragColor = result;
}
