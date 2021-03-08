#version 330 core

out vec4 FragColor;

uniform sampler2D texture_0; // _currentRender
uniform sampler2D texture_1; // normal map
uniform sampler2D texture_2; // mask texture
//uniform sampler2D texture_n;

uniform vec4 screen_factor; // env[0]
uniform vec4 window_coord; // env[1]

in vec4 Color;
in vec2 TexCoords;
in vec2 TexCoords1;
in vec2 TexCoords2;

void main() {
	vec4 mask = texture(texture_2, TexCoords);
	mask = vec4(mask.rg * Color.rg, mask.ba);
	mask = vec4(mask.rg - 0.01, mask.ba);
	// KIL(L)
	if (mask.r < 0.0 || mask.g < 0.0 || mask.b < 0.0 || mask.a < 0.0) {
		discard;
	}

	vec4 local_normal = texture(texture_1, TexCoords1);
	local_normal.x = local_normal.a;
	local_normal = local_normal * 2.0 - 1.0;
	local_normal *= mask;

	vec4 R0 = gl_FragCoord * window_coord;
	R0 = clamp(local_normal * vec4(TexCoords2, 0, 1) + R0, 0.0, 1.0) * screen_factor;
	FragColor = vec4(texture(texture_0, R0.xy).rgb, 1.0);
}
