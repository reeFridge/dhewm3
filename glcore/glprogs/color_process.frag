#version 330 core

out vec4 FragColor;

uniform sampler2D texture_0;
uniform vec4 screen_factor; // env[0]
uniform vec4 window_coord; // env[1]

in vec4 tex_coord_0; // 1 - fraction
in vec4 tex_coord_1; // fraction * target color

void main() {
	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec4 R0 = gl_FragCoord * window_coord;
	// scale by the screen non-power-of-two-adjust
	R0 *= screen_factor;
	// load the screen render
	vec4 texture_color = texture(texture_0, R0.xy);
	// calculate the grey scale version of the color
	R0.r = texture_color.r + texture_color.g + texture_color.b;
	R0 = vec4(R0.r * 0.33);
	// scale by the target color
	R0 *= tex_coord_1;

	FragColor = vec4((texture_color * tex_coord_0 + R0).rgb, 1.0);
}
