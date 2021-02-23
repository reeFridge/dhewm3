#version 330 core

in vec4 ResultColor;
in vec3 FragPos;
in vec3 ViewPos;
in vec3 LightPos;
in vec2 TexCoords;

uniform vec4 diffuse_color_const;
uniform vec4 specular_color_const;

uniform samplerCube t_cube_map;
uniform sampler2D t_bump_map;
uniform sampler2D t_falloff;
uniform sampler2D t_projection;
uniform sampler2D t_diffuse_map;
uniform sampler2D t_specular_map;

in vec4 tc_cube_map;
in vec4 tc_bump_map;
in vec4 tc_falloff;
in vec4 tc_projection;
in vec4 tc_diffuse_map;

out vec4 FragColor;

vec3 calcSpecularComponent(vec3 specular, vec3 norm)
{
	vec3 lightDirection = normalize(LightPos - FragPos);
	vec3 viewDirection = normalize(ViewPos - FragPos);
	vec3 halfwayDir = normalize(viewDirection + lightDirection);
	float shininess = 16.0;
	float spec = pow(max(dot(halfwayDir, norm), 0.0), shininess);
	vec3 specularIntensity = texture(t_specular_map, TexCoords).rgb * 2;
	vec3 specularColor = (specularIntensity * spec) * specular;

	return specularColor;
}

void main()
{
	vec3 light = texture(t_cube_map, tc_cube_map.xyz).rgb * 2.0 - 1.0;
	vec4 local_normal = texture(t_bump_map, tc_bump_map.xy);
	local_normal.x = local_normal.a;
	vec3 normal = vec3(local_normal * 2.0 - 1.0);
	float l = dot(light, normal);
	light = l * vec3(textureProj(t_projection, tc_projection));
	vec3 falloff = vec3(textureProj(t_falloff, tc_falloff));
	if (length(falloff) > 0) {
		light = light * falloff;
	}

	vec3 specular_color = calcSpecularComponent(specular_color_const.xyz, normal);

	vec3 diffuse_color = texture(t_diffuse_map, tc_diffuse_map.xy).xyz * diffuse_color_const.xyz;
	vec3 color = specular_color + diffuse_color;
	color = color * light;

	FragColor = ResultColor * vec4(color, 1.0);
}
