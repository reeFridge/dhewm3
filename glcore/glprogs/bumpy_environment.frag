#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 tc_to_eye;
in vec3 tc_normal;
in vec3 tc_tangent0;
in vec3 tc_tangent1;
in vec4 VertexColor;

uniform samplerCube texture_0; // cube_map
uniform vec4 color;
uniform sampler2D texture_1; // normal_map
uniform bool SVC_IGNORE;

// per-pixel cubic reflextion map calculation

void main() {
	vec4 local_normal = texture(texture_1, TexCoords);
	local_normal.x = local_normal.a;
	local_normal = normalize(local_normal * 2.0 - 1.0);
	mat3 to_tangent_space = transpose(mat3(tc_normal, tc_tangent0, tc_tangent1));
	vec3 global_normal = to_tangent_space * local_normal.xyz;

	vec3 global_eye = normalize(tc_to_eye);
	// calculate reflection vec
	vec3 r0 = ((dot(global_eye, global_normal) * global_normal) * 2.0) - global_eye;

	vec4 in_color = vec4(1.0);

	if (SVC_IGNORE) {
		in_color = color;
	} else {
		in_color = VertexColor;
	}

	vec4 out_color = in_color;
	vec4 texture0_color = texture(texture_0, r0);

	FragColor = vec4(texture0_color.xyz, out_color.a);
}
