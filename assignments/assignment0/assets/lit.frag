#version 450

in Surface{
	vec3 world_pos;
	vec3 world_normal;
	vec2 texcoord;
}vs_out;

uniform sampler2D _MainTexture;

// Light source
uniform vec3 _EyePos;
uniform vec3 _lightDirection = vec3(1.0, 0.0, 0.0);
uniform vec3 _lightColor = vec3(1.0);

out vec4 FragColor;

void main()
{
	vec3 normal = normalize(vs_out.world_normal);

	vec3 toLight = -_lightDirection;

	float diffuseFactor = max(dot(normal, toLight), 0.0);
	vec3 diffuseColor = _lightColor * diffuseFactor;

	// Direction towards eye
	vec3 toEye = normalize(_EyePos - vs_out.world_pos);
	vec3 h = normalize(toLight + toEye);

	float specularFactor = pow(max(dot(normal, h), 0.0), 128);
	vec3 lightColor = (diffuseFactor + specularFactor) * _lightColor;

	vec3 albedo = texture(_MainTexture, vs_out.texcoord).rgb;
	vec3 object_color = albedo * diffuseColor;

	//Shade with 0-1 normal
	//FragColor = vec4(vs_out.normal * 0.5 + 0.5, 1.0f);
	//FragColor = texture(_MainTexture, vs_out.texcoord);
	FragColor = vec4(object_color * lightColor, 1.0);
}