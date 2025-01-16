#version 450

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

uniform sampler2D _MainTexture;

// Light source
uniform vec3 _EyePos;
uniform vec3 _lightDirection = vec3(1.0, 0.0, 0.0);
uniform vec3 _lightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlights
};
uniform Material _Material;

out vec4 FragColor;

void main()
{
	vec3 normal = normalize(vs_out.worldNormal);

	vec3 toLight = -_lightDirection;

	float diffuseFactor = max(dot(normal, toLight), 0.0);
	vec3 diffuseColor = _lightColor * diffuseFactor;

	// Direction towards eye
	vec3 toEye = normalize(_EyePos - vs_out.worldPos);
	vec3 h = normalize(toLight + toEye);

	float specularFactor = pow(max(dot(normal, h), 0.0), _Material.Shininess);
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _lightColor;
	lightColor += _AmbientColor * _Material.Ka;

	vec3 albedo = texture(_MainTexture, vs_out.texcoord).rgb;
	vec3 object_color = albedo * diffuseColor;

	//Shade with 0-1 normal
	//FragColor = vec4(vs_out.normal * 0.5 + 0.5, 1.0f);
	//FragColor = texture(_MainTexture, vs_out.texcoord);
	FragColor = vec4(object_color * lightColor, 1.0);
}