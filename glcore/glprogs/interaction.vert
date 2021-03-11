#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 8) in vec2 aTexCoords;
layout (location = 9) in vec3 aNormal;
layout (location = 10) in vec3 aTangent0;
layout (location = 11) in vec3 aTangent1;

uniform mat4 model;
uniform mat4 modelView;
uniform mat4 proj;

uniform vec4 color_modulate;
uniform vec4 color_add;

uniform vec4 light_origin;
uniform vec4 view_origin;
uniform vec4 light_project_s;
uniform vec4 light_project_t;
uniform vec4 light_project_q;
uniform vec4 light_falloff_s;
uniform vec4 bump_matrix_s;
uniform vec4 bump_matrix_t;
uniform vec4 diffuse_matrix_s;
uniform vec4 diffuse_matrix_t;
uniform vec4 specular_matrix_s;
uniform vec4 specular_matrix_t;

out vec4 ResultColor;

out vec4 tc_cube_map;
out vec4 tc_bump_map;
out vec4 tc_falloff;
out vec4 tc_projection;
out vec4 tc_diffuse_map;
out vec4 tc_specular_map;
out vec4 tc_specular_lookup;

void main()
{
	vec4 defaultTexCoord = vec4(0, 0.5, 0, 1);

	vec3 vl = light_origin.xyz - aPos;
	// order of mat components dictated by old shaders
	mat3 texture_space = transpose(mat3(aTangent0, aTangent1, aNormal));
	tc_cube_map = vec4(texture_space * vl, 0);
	vec3 vln = normalize(vl);
	vec3 vvn = normalize(view_origin.xyz - aPos);
	vec3 halfway = vln + vvn;
	tc_specular_lookup = vec4(texture_space * halfway, 0);

	vec4 tc = vec4(aTexCoords, 0, 1);
	mat4x2 bump_matrix = transpose(mat2x4(bump_matrix_s, bump_matrix_t));
	tc_bump_map = vec4(vec2(bump_matrix * tc), 0, 1);
	mat4x2 diffuse_matrix = transpose(mat2x4(diffuse_matrix_s, diffuse_matrix_t));
	tc_diffuse_map = vec4(vec2(diffuse_matrix * tc), 0, 1);
	mat4x2 specular_matrix = transpose(mat2x4(specular_matrix_s, specular_matrix_t));
	tc_specular_map = vec4(vec2(specular_matrix * tc), 0, 1);

	vec4 p = vec4(aPos, 1.0);
	mat4 light_project_matrix = transpose(mat4(light_project_s, light_project_t, vec4(0), light_project_q));
	tc_projection = light_project_matrix * p;
	tc_falloff = defaultTexCoord;
	tc_falloff.x = dot(p, light_falloff_s);

	gl_Position = proj * modelView * vec4(aPos, 1.0);
	ResultColor = aColor * color_modulate + color_add;
}
