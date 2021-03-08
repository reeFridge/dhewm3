#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
in vec4 VertexColor;

uniform sampler2D texture_0;
uniform vec4 texture_env_color;
uniform vec4 color;

uniform bool SVC_IGNORE;
uniform bool SVC_INVERSE_MODULATE;
uniform bool SECOND_STAGE;

void main() {
	vec4 in_color = vec4(1.0);

	if (SVC_IGNORE) {
		in_color = color;
	} else {
		in_color = VertexColor;
	}

	vec4 out_color = in_color;
	vec4 texture0_color = texture(texture_0, TexCoords);

	if (SVC_INVERSE_MODULATE) {
		out_color = clamp(texture0_color * (vec4(1.0) - in_color), 0.0, 1.0);
	} else {
		out_color *= texture0_color;
	}

	if (SECOND_STAGE) {
		out_color = clamp(out_color * texture_env_color, 0.0, 1.0);
	}

	FragColor = out_color;
}
