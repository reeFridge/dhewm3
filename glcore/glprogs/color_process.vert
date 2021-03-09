#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 modelView;
uniform mat4 proj;

uniform vec4 vertex_parm_0; // parameter 0 is the fraction from the current hue to the target hue to map
uniform vec4 vertex_parm_1; // is the target hue

out vec4 tex_coord_0;
out vec4 tex_coord_1;

void main() {
	gl_Position = proj * modelView * vec4(aPos, 1.0);
	tex_coord_0 = vec4(1.0) - vertex_parm_0;
	tex_coord_1 = vertex_parm_0 * vertex_parm_1;
}
