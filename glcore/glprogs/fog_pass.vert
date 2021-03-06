#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 modelView;
uniform mat4 proj;

uniform vec2 tex_coord_0;
uniform vec2 tex_coord_1;

uniform vec4 object_plane_s_0;
uniform vec4 object_plane_t_0;
uniform vec4 object_plane_s_1;
uniform vec4 object_plane_t_1;

out vec4 TexCoord0;
out vec4 TexCoord1;

void main() {
	vec4 vp = vec4(aPos, 1.0);
	gl_Position = proj * modelView * vp;
	TexCoord0 = vec4(tex_coord_0, 0, 1);
	TexCoord0.x = dot(vp, object_plane_s_0);
	TexCoord0.y = dot(vp, object_plane_t_0);
	TexCoord1 = vec4(tex_coord_1, 0, 1);
	TexCoord1.x = dot(vp, object_plane_s_1);
	TexCoord1.y = dot(vp, object_plane_t_1);
}
