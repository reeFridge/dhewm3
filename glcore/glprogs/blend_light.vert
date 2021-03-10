#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 modelView;
uniform mat4 proj;

uniform vec2 tex_coord_1;

uniform vec4 object_plane_s_0;
uniform vec4 object_plane_t_0;
uniform vec4 object_plane_q_0;

uniform vec4 object_plane_s_1;

uniform bool HAS_TEXTURE_MATRIX;
uniform mat4 texture_matrix;

out vec4 TexCoord0;
out vec4 TexCoord1;

void main() {
	vec4 vp = vec4(aPos, 1.0);
	gl_Position = proj * modelView * vp;
	mat4 light_project_matrix = transpose(mat4(object_plane_s_0, object_plane_t_0, vec4(0), object_plane_q_0));
	TexCoord0 = light_project_matrix * vp;
	TexCoord1 = vec4(tex_coord_1, 0, 1);
	TexCoord1.x = dot(vp, object_plane_s_1);
	if (HAS_TEXTURE_MATRIX) {
		TexCoord0 = texture_matrix * TexCoord0;
		TexCoord1 = texture_matrix * TexCoord1;
	}
}
