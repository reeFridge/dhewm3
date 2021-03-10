#version 330 core

out vec4 FragColor;

uniform sampler2D texture_0; // will get the falloff texture
uniform sampler2D texture_1; // is the entering plane fade correction
uniform vec4 color;

in vec4 TexCoord0;
in vec4 TexCoord1;

void main() {
	vec4 out_color = color;
	out_color *= textureProj(texture_0, TexCoord0);
	out_color *= textureProj(texture_1, TexCoord1);

	FragColor = out_color;
}
