#version 330 core

out vec4 FragColor;
in vec3 Normal; // is the surface normal
in vec3 ToEye; // is toEye, the eye XYZ - the surface XYZ
in vec4 VertexColor;

uniform samplerCube texture_0; // cube_map
uniform bool SVC_INVERSE_MODULATE;
uniform bool SVC_IGNORE;
uniform vec4 color;
uniform vec4 texture_env_color;

// per-pixel cubic reflextion map calculation

void main() {
	vec3 normal = normalize(Normal);
	vec3 toEye = normalize(ToEye);
	// calculate reflection vec
	vec3 r0 = ((dot(toEye, normal) * normal) * 2.0) - toEye;
	vec4 texture_color = texture(texture_0, r0);
	vec4 c = vec4(1.0);
	if (SVC_IGNORE) {
		c = color;
	} else {
		c = VertexColor;
		if (SVC_INVERSE_MODULATE) {
			c = vec4(1.0) - c;
		}
	}

	if (SVC_IGNORE) {
		FragColor = vec4((c * texture_color).xyz, 1.0);
	} else {
		FragColor = c * texture_color * texture_env_color;
	}
}
