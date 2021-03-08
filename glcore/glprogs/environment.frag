#version 330 core

out vec4 FragColor;
in vec3 Normal; // is the surface normal
in vec3 ToEye; // is toEye, the eye XYZ - the surface XYZ
in vec4 VertexColor;

uniform samplerCube texture_0; // cube_map
uniform bool SVC_IGNORE;
uniform vec4 color;

// per-pixel cubic reflextion map calculation

void main() {
	vec3 normal = normalize(Normal);
	vec3 toEye = normalize(ToEye);
	// calculate reflection vec
	vec3 r0 = ((dot(toEye, normal) * normal) * 2.0) - toEye;

	vec4 in_color = vec4(1.0);

	if (SVC_IGNORE) {
		in_color = color;
	} else {
		in_color = VertexColor;
	}

	vec4 out_color = in_color;
	vec4 texture0_color = texture(texture_0, r0);

	out_color *= texture0_color;

	FragColor = out_color;
}
