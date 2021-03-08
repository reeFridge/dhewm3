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

out vec2 TexCoord0;
out vec2 TexCoord1;

void main() {
	vec4 vp = vec4(aPos, 1.0);
	gl_Position = proj * modelView * vp;
	TexCoord0 = vec2(dot(vp, object_plane_s_0), dot(vp, object_plane_t_0));
	TexCoord1 = vec2(dot(vp, object_plane_s_1), dot(vp, object_plane_t_1));
}
