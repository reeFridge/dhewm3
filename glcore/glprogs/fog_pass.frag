#version 330 core

out vec4 FragColor;

uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform vec4 color;

in vec2 TexCoord0;
in vec2 TexCoord1;

void main() {
	vec4 out_color = vec4(color.rgb, 1.0); // glColor3fv
	out_color *= texture(texture_0, TexCoord0);
	out_color *= texture(texture_1, TexCoord1);

	FragColor = out_color;
}
