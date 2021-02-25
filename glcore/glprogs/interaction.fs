#version 330 core

in vec4 ResultColor;

uniform sampler2D t_falloff;

uniform vec4 diffuse_color_const;
uniform vec4 specular_color_const;

uniform samplerCube t_cube_map;
uniform sampler2D t_bump_map;
uniform sampler2D t_projection;
uniform sampler2D t_diffuse_map;
uniform sampler2D t_specular_map;
uniform sampler2D t_specular_lookup;

in vec4 tc_cube_map;
in vec4 tc_bump_map;
in vec4 tc_falloff;
in vec4 tc_projection;
in vec4 tc_diffuse_map;
in vec4 tc_specular_map;
in vec4 tc_specular_lookup;

out vec4 FragColor;

void main()
{
	vec3 light = texture(t_cube_map, tc_cube_map.xyz).rgb * 2.0 - 1.0;
	vec4 local_normal = texture(t_bump_map, tc_bump_map.xy);
	local_normal.x = local_normal.a;
	vec3 normal = vec3(local_normal * 2.0 - 1.0);
	float light_normal = dot(light, normal);
	vec3 projection = textureProj(t_projection, tc_projection).rgb;
	// light intensity
	vec3 falloff = textureProj(t_falloff, tc_falloff).rrr;
	light = (light_normal * projection) * falloff;

	vec3 specular_lookup = vec3(dot(normalize(tc_specular_lookup).xyz, normal));
	// specular intensity
	vec3 specular_falloff = texture(t_specular_lookup, specular_lookup.xy).rrr * specular_color_const.xyz;
	vec3 specular_color = texture(t_specular_map, tc_specular_map.xy).rgb * 2;
	specular_color = specular_color * specular_falloff;
	
	vec3 diffuse_color = texture(t_diffuse_map, tc_diffuse_map.xy).xyz * diffuse_color_const.xyz;

	vec3 color = specular_color + diffuse_color;
	color = color * light;

	FragColor = ResultColor * vec4(color, 1.0);
}
