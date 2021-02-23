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
out vec3 FragPos;
out vec3 ViewPos;
out vec3 LightPos;
out vec2 TexCoords;

out vec4 tc_cube_map;
out vec4 tc_bump_map;
out vec4 tc_falloff;
out vec4 tc_projection;
out vec4 tc_diffuse_map;

void main()
{
	vec4 vertex_position = proj * modelView * vec4(aPos, 1.0);

	vec4 defaultTexCoord = vec4(0, 0.5, 0, 1);
	FragPos = vec3(model * vec4(aPos, 1.0));
	ViewPos = vec3(model * vec4(view_origin.xyz, 1.0));
	LightPos = vec3(model * vec4(light_origin.xyz, 1.0));
	TexCoords = aTexCoords;

	tc_cube_map = vec4(0);
	vec3 vl = light_origin.xyz - aPos;
	tc_cube_map.x = dot(aNormal,   vl);
	tc_cube_map.y = dot(aTangent0, vl);
	tc_cube_map.z = dot(aTangent1, vl);

	tc_bump_map = defaultTexCoord;
	vec4 tc = vec4(aTexCoords, 0, 0);
	tc_bump_map.x = dot(tc, bump_matrix_s);
	tc_bump_map.y = dot(tc, bump_matrix_t);

	vec4 p = vec4(aPos, 1.0);
	tc_falloff = defaultTexCoord;
	tc_falloff.x = dot(p, light_falloff_s);

	tc_projection = vec4(0);
	tc_projection.x = dot(p, light_project_s);
	tc_projection.y = dot(p, light_project_t);
	tc_projection.w = dot(p, light_project_q);

	tc_diffuse_map = defaultTexCoord;
	tc_diffuse_map.x = dot(tc, diffuse_matrix_s);
	tc_diffuse_map.y = dot(tc, diffuse_matrix_t);

	gl_Position = vertex_position;
	ResultColor = aColor * color_modulate + color_add;
}
